function Controller() {
    //installer.autoRejectMessageBoxes();
    installer.installationFinished.connect(function() {
        //gui.clickButton(buttons.NextButton);
    });
}

Controller.prototype.WelcomePageCallback = function() {
    //gui.clickButton(buttons.NextButton, 3000);
}

Controller.prototype.CredentialsPageCallback = function() {
    //gui.clickButton(buttons.NextButton);
}

Controller.prototype.IntroductionPageCallback = function() {
    //gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function() {
    //gui.clickButton(buttons.CommitButton);
}

Controller.prototype.PerformInstallationPageCallback = function() {
    //gui.clickButton(buttons.CommitButton);
}

Controller.prototype.FinishedPageCallback = function() {
    var checkBoxForm = gui.currentPageWidget().LaunchCheckBoxForm;
    if (checkBoxForm && checkBoxForm.launchCheckBox) {
        checkBoxForm.launchCheckBox.checked = false;
    }
    //gui.clickButton(buttons.FinishButton);
}

Controller.prototype.TargetDirectoryPageCallback = function() {
    var targetDir = installer.value("TargetDir");
    if (installer.fileExists(targetDir + "/maintenancetool.exe")) {
        var buttonPressed = QMessageBox["warning"]("os.QMessageBox", "Warning",
            "A previous installation has been detected at " + targetDir +
            ". Do you want to uninstall it?",
            QMessageBox.Yes | QMessageBox.No);

        if (buttonPressed == QMessageBox.Yes) {
            installer.executeDetached(targetDir + "/maintenancetool.exe",
                                      ["--uninstall"]);
        } else {
            //gui.clickButton(buttons.CancelButton);
        }
    }
    else {
        var targetWidget = gui.pageWidgetByObjectName("TargetDirectoryPage");
        targetWidget.TargetDirectoryLineEdit.setText(targetDir);
        //gui.clickButton(buttons.NextButton);
    }
}


