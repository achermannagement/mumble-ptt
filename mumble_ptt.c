#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <libinput.h>
#include <dbus/dbus.h>

#define RIGHT_CTRL_KEY 97
#define LIBINPUT_SEAT "seat0"

static DBusConnection *connection = NULL;
static struct udev *udev = NULL;
static struct libinput *libinput = NULL;

static int open_restricted(const char *path, int flags, void *user_data) {
  return open(path, flags);
}
static void close_restricted(int fd, void *user_data) {
  close(fd);
}

static struct libinput_interface interface = {&open_restricted, &close_restricted};

static int init_udev(void) {
  udev = udev_new();
  if (!udev) {
    return 1;
  }
  return 0;
}

static int init_dbus(void) {
  connection = dbus_bus_get(DBUS_BUS_SESSION, NULL);
  if (!connection) {
    return 2;
  }
  return 0;
}

static int init_libinput(void) {
  libinput = libinput_udev_create_context(&interface, NULL, udev);

  if (!libinput) {
    return 3;
  }

  int result = libinput_udev_assign_seat(libinput, LIBINPUT_SEAT);
  if (result == -1) {
    libinput_unref(libinput);
    return 4;
  }

  return 0;
}

int dbus_send(char *command) {
  DBusError err;
  DBusMessage *msg = dbus_message_new_method_call("net.sourceforge.mumble.mumble", "/", "net.sourceforge.mumble.Mumble", command);
  if (!dbus_connection_send_with_reply_and_block(connection, msg, DBUS_TIMEOUT_USE_DEFAULT, &err)) {
    return 5;
  }
  // ignore the reply (if any)
  //dbus_connection_pop_message(connection);
  dbus_message_unref(msg);
  if (dbus_error_is_set(&err)) {
    dbus_error_free(&err);
    return 6;
  }
  return 0;
}

static void handle_event(struct libinput_event *event) {
  struct libinput_event_keyboard *keyboard_event;

  if (libinput_event_get_type(event) != LIBINPUT_EVENT_KEYBOARD_KEY) {
    return;
  }

  keyboard_event = libinput_event_get_keyboard_event(event);

  uint32_t key = libinput_event_keyboard_get_key(keyboard_event);
  int key_state = libinput_event_keyboard_get_key_state(keyboard_event);
  if (key == RIGHT_CTRL_KEY) {
    if (key_state == LIBINPUT_KEY_STATE_PRESSED) {
      dbus_send("startTalking");
    } else if (key_state == LIBINPUT_KEY_STATE_RELEASED) {
      dbus_send("stopTalking");
    }
  }
}

int main(void) {
  int errno = init_dbus();
  if (errno != 0) {
    return errno;
  }
  errno = init_udev();
  if (errno != 0) {
    // dbus connection MUST have succeeded in previous conditional
    dbus_connection_unref(connection);
    return errno;
  }
  errno = init_libinput();
  if (errno != 0) {
    // udev init MUST have succeeded in previous conditional
    udev_unref(udev);
    // dbus connection MUST have succeeded in previous conditional
    dbus_connection_unref(connection);
    return errno;
  }

  // main loop
  while (1) {
    struct libinput_event *event = libinput_get_event(libinput);
    struct pollfd fd = {libinput_get_fd(libinput), POLLIN, 0};
    poll(&fd, 1, -1);
    libinput_dispatch(libinput);

    while((event = libinput_get_event(libinput))) {
      handle_event(event);
    }

    libinput_event_destroy(event);
  }

  if (libinput) {
    libinput_unref(libinput);
  }
  if (udev) {
    udev_unref(udev);
  }
  if (connection) {
    dbus_connection_unref(connection);
  }

  return 0;
}


