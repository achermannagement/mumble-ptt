#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <libinput.h>
#include <libudev.h>

#define RIGHT_CTRL_KEY 97
#define LIBINPUT_SEAT "seat0"

static struct udev *udev = NULL;
static struct libinput *libinput = NULL;

static int open_restricted(const char *path, int flags, __attribute__((unused)) void *user_data) {
  return open(path, flags);
}
static void close_restricted(int fd, __attribute__((unused)) void *user_data) {
  close(fd);
}
static const struct libinput_interface interface = {
  .open_restricted = open_restricted,
  .close_restricted = close_restricted,
};

static int init_udev(void) {
  udev = udev_new();
  if (!udev) {
    return 1;
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

int dbus_send_start(void) {
  return system("dbus-send --session --type=method_call --dest=net.sourceforge.mumble.mumble / net.sourceforge.mumble.Mumble.startTalking");
}

int dbus_send_stop(void) {
  return system("dbus-send --session --type=method_call --dest=net.sourceforge.mumble.mumble / net.sourceforge.mumble.Mumble.stopTalking");
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
      dbus_send_start();
    } else if (key_state == LIBINPUT_KEY_STATE_RELEASED) {
      dbus_send_stop();
    }
  }
  libinput_event_destroy(event);
}

// when our poll fires we need to handle all events
static void handle_events() {
    struct libinput_event *event;
    while((event = libinput_get_event(libinput)) != NULL) {
      handle_event(event);
    }
}

int main(void) {
  int errno = init_udev();
  if (errno != 0) {
    udev_unref(udev);
    return errno;
  }
  errno = init_libinput();

  if (errno != 0) {
    // udev init MUST have succeeded in previous conditional
    udev_unref(udev);
    return errno;
  }

  struct pollfd fd = {
    .fd = libinput_get_fd(libinput),
    .events = POLLIN,
    .revents = 0
  };

  // main loop
  while (poll(&fd, 1, -1) > -1) {
    libinput_dispatch(libinput);
    handle_events();
  }

  if (libinput) {
    libinput_unref(libinput);
  }
  if (udev) {
    udev_unref(udev);
  }
  return 0;
}


