
# Control UI

## Install

### Python dependencies

    sudo pip install Flask

### Automatic startup

Run start-ambi-tv (to make sure the pid is set to /var/run) and control.py via initd

```
/etc/inittab
al:2:respawn:/home/pi/ambi-tv/bin/start-ambi-tv
ac:2:respawn:/usr/bin/sudo -u pi /usr/bin/python /home/pi/ambi-tv/control/control.py
```

    sudo telinit q

#### PID detection

The script ``bin/start-ambi-tv`` should be used to start the app so the pid is
noted.

## URL

The default url is http://ambi-tv-host:5000/

