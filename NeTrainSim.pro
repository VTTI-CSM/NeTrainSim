TEMPLATE = subdirs

# Assign alias to your subdir projects
NeTrainSim.subdir = src/NeTrainSim
NeTrainSimGUI.subdir = src/NeTrainSimGUI
NeTrainSimInstaller.subdir = src/NeTrainSimInstaller

# Set up the dependencies
NeTrainSimGUI.depends += NeTrainSim
NeTrainSimInstaller.depends += NeTrainSim NeTrainSimGUI

# Add them to the build order
SUBDIRS += NeTrainSim NeTrainSimGUI NeTrainSimInstaller

# Enable parallel building
CONFIG += ordered

OTHER_FILES += \
    README.md
