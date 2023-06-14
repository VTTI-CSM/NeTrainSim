TEMPLATE = subdirs


# Assign alias to your subdir projects
NeTrainSim.subdir = src/NeTrainSim
NeTrainSimGUI.subdir = src/NeTrainSimGUI
NeTrainSimInstaller.subdir = src/NeTrainSimInstaller

# Add them to the build order
SUBDIRS = NeTrainSimInstaller NeTrainSim NeTrainSimGUI

# Set up the dependencies
NeTrainSimGUI.depends = NeTrainSim
NeTrainSimInstaller.depends = NeTrainSim NeTrainSimGUI

# Enable parallel building
CONFIG += ordered

OTHER_FILES += \
    README.md
