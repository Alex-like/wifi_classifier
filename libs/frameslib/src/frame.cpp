//
// Created by Alex Shchelochkov on 22.09.2022.
//

#include "../include/frame.hpp"

using namespace std;

frameslib::frames::LogFrame::LogFrame() = default;

frameslib::frames::LogFrame::LogFrame(uint64_t ind_v, double Offset_v, string BW_v, string MCS_v, uint32_t Size_v,
                                       string Frame_v,
                                       string info_v, bool FCS_v, tl::optional<string> Type_v, tl::optional<string> SSID_v,
                                       tl::optional<uint64_t> TA_v, tl::optional<uint64_t> RA_v, tl::optional<bool> moreFragments_v,
                                       tl::optional<uint64_t> seqNum_v, tl::optional<uint64_t> fragNum_v) {
    ind = ind_v;
    Offset = Offset_v;
    BW = move(BW_v);
    MCS = move(MCS_v);
    Size = Size_v;
    Frame = move(Frame_v);
    info = move(info_v);
    FCS = FCS_v;
    Type = move(Type_v);
    SSID = move(SSID_v);
    TA = TA_v;
    RA = RA_v;
    moreFragments = moreFragments_v;
    seqNum = seqNum_v;
    fragNum = fragNum_v;
}

uint64_t frameslib::frames::LogFrame::getInd() const {
    return ind;
}

string frameslib::frames::LogFrame::getBW() {
    return BW;
}

string frameslib::frames::LogFrame::getMCS() {
    return MCS;
}

string frameslib::frames::LogFrame::getFrame() {
    return Frame;
}

bool frameslib::frames::LogFrame::isCorrect() const {
    return FCS;
}

string frameslib::frames::LogFrame::toString() {
    stringstream s;
    s << "Index=" << ind << " FCS=" << (isCorrect() ? "Success" : "Fail") << endl;
    return s.str();
}

string frameslib::frames::LogFrame::getTAAndRA() {
    string TA_str = TA.has_value() ? to_string(TA.value()) : " –";
    string RA_str = RA.has_value() ? to_string(RA.value()) : " –";
    return "TA=" + TA_str + " RA=" + RA_str;
}

tl::optional<string> frameslib::frames::LogFrame::getType() {
    return Type;
}

tl::optional<string> frameslib::frames::LogFrame::getSSID() {
    return SSID;
}

tl::optional<uint64_t> frameslib::frames::LogFrame::getTA() {
    return TA;
}

tl::optional<uint64_t> frameslib::frames::LogFrame::getRA() {
    return RA;
}

string frameslib::frames::LogFrame::getData() {
    return info;
}

uint32_t frameslib::frames::LogFrame::getSize() const {
    return Size;
}

bool frameslib::frames::LogFrame::getMoreFrags() {
    return moreFragments.has_value() && moreFragments.value();
}

uint64_t frameslib::frames::LogFrame::getSeqNum() {
    return seqNum.has_value() ? seqNum.value() : 0;
}

uint64_t frameslib::frames::LogFrame::getFragNum() {
    return fragNum.has_value() ? fragNum.value() : 0;
}

double frameslib::frames::LogFrame::getOffset() const {
    return Offset;
}

void frameslib::frames::LogFrame::setOffset(double offset) {
    this->Offset = offset;
}

void frameslib::frames::LogFrame::setBW(const string& bw) {
    this->BW = bw;
}

void frameslib::frames::LogFrame::setMCS(const string& mcs) {
    this->MCS = mcs;
}

void frameslib::frames::LogFrame::setSize(uint32_t size) {
    this->Size = size;
}

void frameslib::frames::LogFrame::setFrame(const string& frame) {
    this->Frame = frame;
}

void frameslib::frames::LogFrame::setData(const string& new_info) {
    this->info = new_info;
}

void frameslib::frames::LogFrame::setType(const tl::optional<string>& type) {
    this->Type = type;
}

void frameslib::frames::LogFrame::setSSID(const tl::optional<string>& ssid) {
    this->SSID = ssid;
}

void frameslib::frames::LogFrame::setTA(const tl::optional<uint64_t> ta) {
    this->TA = ta;
}

void frameslib::frames::LogFrame::setRA(const tl::optional<uint64_t> ra) {
    this->RA = ra;
}

void frameslib::frames::LogFrame::setFCS(const bool fcs) {
    this->FCS = fcs;
}

void frameslib::frames::LogFrame::setMoreFrags(const tl::optional<bool> moreFrags) {
    this->moreFragments = moreFrags;
}

void frameslib::frames::LogFrame::setSeqNum(const tl::optional<uint64_t> new_seqNum) {
    this->seqNum = new_seqNum;
}

void frameslib::frames::LogFrame::setFragNum(const tl::optional<uint64_t> new_fragNum) {
    this->fragNum = new_fragNum;
}

frameslib::frames::LogFrame frameslib::frames::parse(const vector<string> &lines,
               bool hasHeader,
               bool hasBody,
               bool oldFileFormat) {
    // init
    double Offset_v = 0.0;
    string BW_v, MCS_v, Frame_v, info_v;
    uint64_t ind_v = 1;
    uint32_t Size_v = 0;
    tl::optional<uint64_t> TA_v, RA_v, seqNum, fragNum;
    tl::optional<string> Type_v, SSID_v;
    bool FCS_v = false, moreFrags= false;

    // parse
    const string MAC_template = "([0-9a-fA-F]+):([0-9a-fA-F]+):([0-9a-fA-F]+):([0-9a-fA-F]+):([0-9a-fA-F]+):([0-9a-fA-F]+)";
    // regular expressions for parsing DataFrame
    const regex regex_ind("^(\\d+)");
    const regex regex_offset(R"(Offset=([0-9]+(\.[0-9]+)?))");
    const regex regex_BW(R"(BW=([^,]+))");
    const regex regex_MCS(R"(MCS=([^,]+))");
    const regex regex_Size(R"(Size=([0-9]+))");
    const regex regex_line2_Frame("Frame=([0-9a-zA-Z]+)");
    const regex regex_line2_Bits("Bits=([0-9a-zA-Z]+)");
    const regex regex_FCS("FCS=Fail");
    const regex regex_Type("Type=([^,]+),");
    const regex regex_MoreFrags("More Fragments=([0-9]+)");
    const regex regex_SSID("[^B]SSID=('.*')");
    const regex regex_TA("TA.*?=" + MAC_template);
    const regex regex_RA("RA.*?=" + MAC_template);
    const regex regex_Seqnum("Seqnum=([0-9]+)");
    const regex regex_Fragnum("Fragnum=([0-9]+)");

    smatch ind;
    if (regex_search(lines[0], ind, regex_ind))
        ind_v = stoull(ind[1].str());

    if (hasHeader) {
        smatch line1_groups;
        if (regex_search(lines[0], line1_groups, regex_offset)) {
            Offset_v = stod(line1_groups[1].str());
        }
        if (regex_search(lines[0], line1_groups, regex_BW)) {
            BW_v = line1_groups[1].str();
        }
        if (regex_search(lines[0], line1_groups, regex_MCS)) {
            MCS_v = line1_groups[1].str();
        }
        if (regex_search(lines[0], line1_groups, regex_Size)) {
            Size_v = stoul(line1_groups[1].str());
        }

        smatch line2_groups;
        if (regex_search(lines[oldFileFormat ? 1 : 0], line2_groups, regex_line2_Frame) ||
            regex_search(lines[oldFileFormat ? 1 : 0], line2_groups, regex_line2_Bits)) {
            Frame_v = line2_groups[1].str();
        }
    }

    if (hasBody) {
        smatch line3_groups;
        if (!regex_search(lines[oldFileFormat ? 2 : (hasHeader ? 1 : 0)], line3_groups, regex_FCS)) {
            FCS_v = true;
        }
        info_v = lines[oldFileFormat ? 2 : (hasHeader ? 1 : 0)];
        if (FCS_v && regex_search(info_v, line3_groups, regex_Type)) {
            Type_v = line3_groups[1].str();
        }
        if (FCS_v && regex_search(info_v, line3_groups, regex_SSID)) {
            SSID_v = line3_groups[1].str();
        }
        if (FCS_v && regex_search(info_v, line3_groups, regex_TA)) {
            string hex = line3_groups[1].str() + line3_groups[2].str() + line3_groups[3].str() + line3_groups[4].str() + line3_groups[5].str() + line3_groups[6].str();
            TA_v = stoull(hex, nullptr, 16);
        }
        if (FCS_v && regex_search(info_v, line3_groups, regex_RA)) {
            string hex = line3_groups[1].str() + line3_groups[2].str() + line3_groups[3].str() + line3_groups[4].str() + line3_groups[5].str() + line3_groups[6].str();
            RA_v = stoull(hex, nullptr, 16);
        }
        if (FCS_v && regex_search(info_v, line3_groups, regex_MoreFrags)) {
            string flag = line3_groups[1].str();
            int tmp = stoi(flag);
            moreFrags = tmp > 0;
        }
        if (FCS_v && regex_search(info_v, line3_groups, regex_Seqnum)) {
            string tmp = line3_groups[1].str();
            seqNum = stoull(tmp);
        }
        if (FCS_v && regex_search(info_v, line3_groups, regex_Fragnum)) {
            string tmp = line3_groups[1].str();
            fragNum = stoull(tmp);
        }
    }

    // return
    return {ind_v, Offset_v, BW_v, MCS_v, Size_v, Frame_v, info_v, FCS_v, Type_v, SSID_v, TA_v, RA_v, moreFrags, seqNum, fragNum};
}

int frameslib::frames::readFromFile(const string &path,
                                     vector<LogFrame> &to,
                                     bool hasHeader,
                                     bool hasBody,
                                     bool oldFileFormat) {
    int count = 0;
    int expectedCount = 0;
    if (hasHeader) expectedCount++;
    if (hasBody) expectedCount++;
    if (oldFileFormat) expectedCount++;

    vector<string> lines(expectedCount);
    ifstream in(path);
    if (in.is_open()) {
        string line;
        while (getline(in, line, '\n')) {
            // skip empty string
            smatch match;
            if (regex_match(line, match, regex("^\\s*$"))) {
                continue;
            }

            // remember string
            lines[count++] = line;
            if (count == expectedCount) {
                to.emplace_back(parse(lines, hasHeader, hasBody, oldFileFormat));
                count = 0;
            }
        }
    } else return -1;
    in.close();
    if (0 < count && count < expectedCount) return -2;
    return 0;
}
