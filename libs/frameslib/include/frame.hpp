//
// Created by Alex Shchelochkov on 22.09.2022.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include "optional.hpp"

namespace frameslib { namespace frames {
    /// Frame structure. Contains all information about frame.
    class LogFrame {
    private:
        // 1 Line
        /// Frame index.
        uint64_t ind;
        /// Frame offset value.
        double Offset;
        /// Bandwidth.
        std::string BW;
        /// Modulation and Coding Scheme.
        std::string MCS;
        /// Frame size.
        uint32_t Size;
        // 2 Line
        /// Hexadecimal data representation.
        std::string Frame;
        // 3 Line
        /// Decoded data.
        std::string info;
        /// Frame type if frame is correct.
        tl::optional<std::string> Type;
        /// Frame SSID if frame is correct. Like name.
        tl::optional<std::string> SSID;
        // MAC addresses
        /// TA - transmitter address, if frame is correct.
        tl::optional<uint64_t> TA;
        /// RA - receiver address, if frame is correct.
        tl::optional<uint64_t> RA;
        /// Correctness of frame.
        bool FCS;
        /// More fragments flag.
        tl::optional<bool> moreFragments;
        /// Sequence number.
        tl::optional<uint64_t> seqNum;
        /// Fragment number.
        tl::optional<uint64_t> fragNum;
    public:
        /// Default constructor.
        LogFrame();
        /**
         * Basic constructor.
         *
         * @param ind_v frame index.
         * @param Offset_v offset value.
         * @param BW_v bandwidth.
         * @param MCS_v modulation and coding scheme.
         * @param Size_v frame size.
         * @param Frame_v hexadecimal value of data.
         * @param info_v decoded data.
         * @param FCS_v frame correctness.
         * @param Type_v frame type if frame is correct.
         * @param SSID_v frame name if frame is correct.
         * @param TA_v transmitter address if frame is correct and has it.
         * @param RA_v receiver address if frame is correct and has it.
         */
        LogFrame(uint64_t ind_v, double Offset_v, std::string BW_v, std::string MCS_v, uint32_t Size_v,
                 std::string Frame_v,
                 std::string info_v, bool FCS_v, tl::optional<std::string> Type_v, tl::optional<std::string> SSID_v,
                 tl::optional<uint64_t> TA_v, tl::optional<uint64_t> RA_v, tl::optional<bool> moreFragments_v,
                 tl::optional<uint64_t> seqNum_v, tl::optional<uint64_t> fragNum_v);
        /**
         * Get frame's index.
         *
         * @return index.
         */
        uint64_t getInd() const;
        /**
         * Get frame's BW.
         *
         * @return BW value.
         */
        std::string getBW();
        /**
         * Get frame's MCS.
         *
         * @return MCS value.
         */
        std::string getMCS();
        /**
         * Get hexadecimal frame's data.
         *
         * @return hexadecimal representation of frame.
         */
        std::string getFrame();
        /**
         * Get correctness of frame.
         *
         * @return frame correctness.
         */
        bool isCorrect() const;
        /**
         * Convert information to string value.
         *
         * @return string representation of some information about frame.
         */
        std::string toString();
        /**
         * Convert TA and RA to string value.
         *
         * @return string representation of TA and RA.
         */
        std::string getTAAndRA();
        /**
         * Get frame type.
         *
         * @return frame type if frame is correct.
         */
        tl::optional<std::string> getType();
        /**
         * Get transmitter address.
         *
         * @return TA if frame is correct and has it.
         */
        tl::optional<uint64_t> getTA();
        /**
         * Get receiver address.
         *
         * @return RA if frame is correct and has it.
         */
        tl::optional<uint64_t> getRA();
        /**
         * Get frame SSID.
         *
         * @return SSID if frame is correct and has it.
         */
        tl::optional<std::string> getSSID();
        /**
         * Get decoded data of frame.
         *
         * @return decoded frame.
         */
        std::string getData();
        /**
         * Get frame size.
         *
         * @return number of transmitted bytes.
         */
        uint32_t getSize() const;
        /**
         * Get flag of existing more fragments of current frame.
         *
         * @return MoreFragments flag.
         */
        bool getMoreFrags();
        /**
         * Get sequence number of frame.
         *
         * @return sequence number.
         */
        uint64_t getSeqNum();
        /**
         * Get fragment number of frame.
         *
         * @return fragment number.
         */
        uint64_t getFragNum();
        /**
         * Get time offset of current frame.
         *
         * @return time offset.
         */
        double getOffset() const;
        /**
         * Sst offset value.
         *
         * @param offset value.
         */
        void setOffset(double offset);
        /**
         * Set BW value.
         *
         * @param bw value.
         */
        void setBW(const std::string& bw);
        /**
         * Set MCS value.
         *
         * @param mcs value.
         */
        void setMCS(const std::string& mcs);
        /**
         * Set Size of frame.
         *
         * @param size value.
         */
        void setSize(uint32_t size);
        /**
         * Set bits representation of decoded information.
         *
         * @param frame hexadecimal string.
         */
        void setFrame(const std::string& frame);
        /**
         * Set decoded data.
         *
         * @param new_info decoded frame.
         */
        void setData(const std::string& new_info);
        /**
         * Set frame type.
         *
         * @param type of frame.
         */
        void setType(const tl::optional<std::string>& type);
        /**
         * Set SSID of frame.
         *
         * @param ssid string.
         */
        void setSSID(const tl::optional<std::string>& ssid);
        /**
         * Set TA of frame.
         *
         * @param ta MAC-address.
         */
        void setTA(tl::optional<uint64_t> ta);
        /**
         * Set RA of frame.
         *
         * @param ra MAC-address.
         */
        void setRA(tl::optional<uint64_t> ra);
        /**
         * Set FCS flag of frame.
         *
         * @param fcs flag.
         */
        void setFCS(bool fcs);
        /**
         * Set MoreFragments flag of frame.
         *
         * @param moreFrags flag.
         */
        void setMoreFrags(tl::optional<bool> moreFrags);
        /**
         * Set Seqnum of frame.
         *
         * @param new_seqNum value.
         */
        void setSeqNum(tl::optional<uint64_t> new_seqNum);
        /**
         * Set Fragnum of frame.
         *
         * @param new_fragNum value.
         */
        void setFragNum(tl::optional<uint64_t> new_fragNum);
    };
    /**
     * Get frame information from string.
     *
     * @param lines reference to separated lines which contain information.
     * @param hasHeader flag to indicate existence of header.
     * @param hasBody flag to indicate existence of decoded body.
     *
     * @return extracted information about frame.
     */
    LogFrame parse(const std::vector<std::string> &lines,
                   bool hasHeader = true,
                   bool hasBody = true,
                   bool oldFileFormat = true);
    /**
     * Fill vector with LogFrames from file.
     *
     * @param path the file to read;
     * @param to reference to filling vector.
     *
     * @return exit code.
     */
    int readFromFile(const std::string &path,
                     std::vector<LogFrame> &to,
                     bool hasHeader = true,
                     bool hasBody = true,
                     bool oldFileFormat = true);
} }