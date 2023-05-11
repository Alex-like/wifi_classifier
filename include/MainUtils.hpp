//
// Created by Alex Shchelochkov on 12.07.2022.
//
#pragma once

#include "libs/frameslib/include/statistics.hpp"
#include "libs/frameslib/include/worker.hpp"
#include "libs/frameslib/include/syncworker.hpp"

#include "GlobalSource.hpp"

namespace WiFiClassifier {
/**
 * Get statistic of processed frames.
 *
 * @param frames reference to stream of frames.
 */
    void getStatistics(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Get amout of frames with different types.
 *
 * @param frames reference to stream of frames.
 */
    void countFrameTypes(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Make graph with dependences of MAC-addresses.
 *
 * @param frames reference to stream of frames.
 */
    void makeSimpleGraph(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Grouped MAC-dependences by networks.
 *
 * @param frames reference to stream of frames.
 */
    void getNetworks(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Get list of MAC-addresses from processed frames.
 *
 * @param frames reference to stream of frames.
 */
    void getAddressesFromFrames(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Get list of SSIDs from processed frames.
 *
 * @param frames reference to stream of frames.
 */
    void getSSIDFromFrames(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Get MAC-address classification.
 *
 * @param frames reference to stream of frames.
 */
    void classifyAllMacFromFrames(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Output types of processed frames.
 *
 * @param frames reference to stream of frames.
 */
    void printFramesTypes(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Filter data-frames, exclude features from them and make dataset with features.
 *
 * @param frames reference to stream of frames.
 */
    void excludeDataForTrainingSet(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Filter data-frames and process them.
 *
 * @param frames reference to stream of frames.
 */
    void workWithDataFrames(std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Process single file with frames.
 *
 * @param path file's path;
 * @param action worker with frames.
 */
    void workWithDefiniteFile(const std::string &path,
                              const std::function<void(std::vector < frameslib::frames::LogFrame > &)> &action);
/**
 * Process all files from source.
 *
 * @param action worker with frames for each file.
 */
    void workWithAllFiles(const std::function<void(std::vector<frameslib::frames::LogFrame> &)>& action);
/**
 * Process all files with separated frames.
 *
 * @param action worker with frames for each file.
 */
    void workWithSeparatedFiles(const std::function<void(std::vector<frameslib::frames::LogFrame> &)>& action);
/**
 * Make a report for work.
 */
    void reportTaskWriter();
/**
 * Make report about additional files for work.
 */
    void reportAdditionalTaskWriter();
/**
 * Read frames from separated files.
 *
 * @param headerPath file with frame's headers;
 * @param bodyPath file with frame's bodies;
 * @param frames reference to reading frames.
 * @return error code.
 */
    int readFramesFromSeparatedFiles(const std::string &headerPath, const std::string &bodyPath,
                                     std::vector<frameslib::frames::LogFrame> &frames);
/**
 * Read frames from separated file and write to single.
 *
 * @param path_1
 * @param path_2
 * @param output_path
 * @return error code.
 */
    int transformer(const std::string& path_1, const std::string& path_2,
                    const std::string& output_path);
}