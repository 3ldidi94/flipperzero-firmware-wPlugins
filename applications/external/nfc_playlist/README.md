# FlipperZero_NFC_Playlist:
The idea behind this app is to allow for you to test multiple copies of NFC's at once as a bulk test

At the moment this project is very basic and just reads the cards from the txt file and emulates the NFC's but this is very early days of development and i wanted to make a basic version of my idea first

## How it works (As of the moment will change):
When you start the app there is a start button when pressed starts the playlist

To set the playlist you need to create a file in ext/apps_data/nfc_playlist called playlist.txt you can find an example of the file in the repository

## Know problems:
- Unable to press any buttons while emulating as they are unresponsive and if you spam them it crashes the app (cause has been located and a fix is being made)
- Emulating UI can be a bit janky not displaying as intended 100% of the time

## TODO:
- [x] Add basic UI
- [X] Add checks for files that are not NFC and missing file and add a display for them
- [X] Add the ability to adjust time between cards
- [ ] Add the ability to adjust card queue from within the app
- [ ] Make it so you can pause, stop and continue the running of the cards
