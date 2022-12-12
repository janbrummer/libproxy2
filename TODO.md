# TODO List

## Step 1
- Architecture review
    - Why network connectivity modules?
        - Network changes for wpad, flushes cache and reforces pac download
            nm module -> TODO

    - Get Config vs Set Config on demand...
        - Get Config due to wpad!

    - ZSCaler??
        config module
    
    - Performance:
        Config loaded: 0.000065
        PAC loaded   : 0.011883
        PAC set      : 0.014091
        PAC parsed   : 0.016664

    - Session to System Daemon communication?
    
    - icon?
        -> Dom fragt nach

- Check error handling

- Async API
    -> Yes -> Use Case

- Shutdown handling!

- Chemnitzer Linux Tage?
    -> Yes :)

## Step 2
- Add missing modules
- Check Windows
- Check OS X
- Check BSD

## Step 3
- Reach out and add patches to:
    - wget
    - curl
    - Python (requests/pip)
