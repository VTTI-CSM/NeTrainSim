/**
 * @file    ~\NeTrainSim\src\util\Error.h
 *
 * Declares the Error enum class.
 */
#ifndef ERROR_H
#define ERROR_H

/**
 * @brief Enumerates the possible error values.
 */
enum class Error {
    // base starts with 0
    cannotRetrieveHomeDir           = 010,

    // nodes start with 1
    nodesFileDoesNotExist           = 100,
    emptyNodesFile                  = 110,
    wrongNodesFileStructure         = 120,
    // links start with 1
    linksFileDoesNotExist           = 130,
    emptyLinksFile                  = 140,
    wrongLinksFileStructure         = 150,
    wrongLinksLength                = 160,
    // network processing start with 1
    cannotFindNode                  = 170,


    // trains start with 2
    trainsFileDoesNotExist          = 200,
    emptyTrainsFile                 = 210,
    wrongTrainsFileStructure        = 220,
    otherTrainsFileErrors           = 230,
    trainPathCannotBeNull           = 240,
    // trains processing starts with
    trainDoesNotHaveLocos           = 250,
    trainHasWrongLocos              = 260,
    trainHasWrongCars               = 270,
    trainInvalidGradesCurvature     = 280,
    trainsFileEmpty                 = 290,

    trainWrongCarType               = 300,
    trainWrongLocoType              = 310,

    // output start with 4
    cannotOpenTrajectoryFile        = 410,
    cannotOpenSummaryFile           = 420,


    CouldNotOpenFile                = 500






};

#endif //ERROR_H
