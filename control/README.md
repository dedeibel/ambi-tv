
# Control UI

Provides a web UI using python flask to control the ambi-tv programs and to shut it down properly. It is using
a fifo `/var/run/ambi-tv/control_fifo` to communicate with ambi-tv.

ambi-tv main is listening to the fifo and expecting the following commands without newline

- NEXT_PROGRAM
- PAUSE

Shutdown is realized by simply calling the following. Ain't pretty but does the trick.

        call(["sudo", "shutdown", "-h", "now"])


## Install

The directory `/var/run/ambitv/` has to be created. Give read and write permission to the user running ambi-tv. Writing the pid to `/var/run/ambitv/pid` is also recommended.

To automate all this simply use the systemd service files and `bin/ambitv-daemon` start script.

### Python dependencies

    sudo apt-get install python-flask
    
### Automatic startup

Use systemd service files as described in the main README, or just see them in [../systemd](../systemd).

## URL

The default url is http://ambi-tv-host:5000/

