TEMPLATE = subdirs

#SUBDIRS += \
#    src/NeTrainSim \
#    src/NeTrainSimGUI \
#    src/dependencies/QtRptProject

# Assign alias to your subdir projects
NeTrainSim.subdir = src/NeTrainSim
NeTrainSimGUI.subdir = src/NeTrainSimGUI
#QtRpt.subdir = src/dependencies/QtRptProject

# Add them to the build order
SUBDIRS = NeTrainSim NeTrainSimGUI

# Set up the dependencies
NeTrainSimGUI.depends = NeTrainSim

# Enable parallel building
CONFIG += ordered

OTHER_FILES += \
    README.md
