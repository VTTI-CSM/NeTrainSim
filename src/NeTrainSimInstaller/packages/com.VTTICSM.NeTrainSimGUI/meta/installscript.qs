function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install README.txt!
    component.createOperations();

    if (systemInfo.productType === "windows") {
        // Create a shortcut in the Start Menu
        component.addOperation("CreateShortcut",
                               "@TargetDir@/NeTrainSimGUI.exe",
                               "@StartMenuDir@/NeTrainSimGUI.lnk",
                               "workingDirectory=@TargetDir@",
                               "description=NeTrainSimGUI");

        // Create a shortcut on the Desktop
        component.addOperation("CreateShortcut",
                               "@TargetDir@/NeTrainSimGUI.exe",
                               "@DesktopDir@/NeTrainSimGUI.lnk",
                               "workingDirectory=@TargetDir@",
                               "description=NeTrainSimGUI");
    }
}

