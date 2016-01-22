# QtInstaShare

The app stays in the system tray and offers three ways to upload content to a webserver.

### Upload clipboard
The app tries to analyze the clipboard contents and upload accordingly.<br>Currently supports text, images and files in clipboard.

### Upload screenshot
The app supports multiple screens and allows selection of which screen to upload.

### Upload file
Self-explanatory.

### Details
If the upload takes longer then defined time (default: 5 sec), a progress dialog is shown.

After the upload is complete a dialog with url is shown. It is enough to copy the link and focus out of the dialog, it will close itself.

### Configuration
To configure the server and secret word, copy the `config.example.json` file to `config.json` and edit it accordingly.

### Building
    git clone https://github.com/xil-se/QtInstaShare.git && cd QtInstaShare
    qmake
    make -j