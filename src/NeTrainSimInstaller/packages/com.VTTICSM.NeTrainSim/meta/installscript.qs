function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    // call the default implementation
    component.createOperations();

    if (systemInfo.productType === "windows") {
        // Create a shortcut in the Start Menu
        component.addOperation("CreateShortcut", "@TargetDir@/NeTrainSim.exe", "@StartMenuDir@/NeTrainSim.lnk",
                    "workingDirectory=@TargetDir@", "description=NeTrainSim");
    }
//    try {
//        component.createOperations();
//        if (installer.value("os") === "win") {
//            var installPath = installer.value("TargetDir").replace(/\//g, '\\');
//            component.addElevatedOperation("Execute", "{0,3010}", "@TargetDir@\\..\\vcredist_x64.exe", "/install /passive /norestart");
//            component.addElevatedOperation("EnvironmentVariable", "append", "PATH", installPath);
//        }
//        if (installer.value("os") === "x11" || installer.value("os") === "mac") {
//            var installPath = installer.value("TargetDir");
//            component.addElevatedOperation("Execute", "/bin/sh", "-c", "echo 'export PATH=$PATH:" + installPath + "' >> ~/.bash_profile");
//        }
//    } catch (e) {
//        print(e);
//    }
}
