function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install README.txt!
    component.createOperations();

    if (systemInfo.productType === "windows") {

        // Start menu shortcut
        component.addOperation("CreateShortcut", 
                               "@TargetDir@/Serious-Runner/Serious-Runner.exe", 
                               "@StartMenuDir@/Serious Runner/Serious-Runner.lnk", 
                               "workingDirectory=@TargetDir@/Serious-Runner", 
                               "iconPath=@TargetDir@/Serious-Runner/Serious-Runner.ico");

       // Desktop Shortcut
       component.addOperation("CreateShortcut", 
                              "@TargetDir@/Serious-Runner/Serious-Runner.exe",
                              "@DesktopDir@/Serious Runner.lnk",
                              "workingDirectory=@TargetDir@/Serious-Runner", 
                              "iconPath=@TargetDir@/Serious-Runner/Serious-Runner.ico");			
    }
}
