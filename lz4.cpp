#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

using namespace std;

vector<uint8_t> setArray(vector<uint8_t> &targetArray, vector<uint8_t> &sourceArray, int &offset) {
    if (targetArray.size() >= sourceArray.size() + offset) {
        for (int i = 0; i < sourceArray.size(); i ++) {
            targetArray[i + offset] = sourceArray[i];
        }
    }
    return targetArray;
}

vector<uint8_t> copyWithin(vector<uint8_t> &array, int targetIndex, int startIndex, int endIndex) {
    /*if (startIndex < 0) {
        startIndex += array.size();
    }
    if (endIndex < 0) {
        endIndex += array.size();
    }*/
    //vector<uint8_t> sourceArray(array.begin() + startIndex, array.begin() + endIndex);
    //vector<uint8_t> beforeArray(array.begin(), array.begin() + targetIndex);
    //vector<uint8_t> afterArray(array.begin() + targetIndex, array.end());
    //cout << sourceArray.size() << endl;

    //memcpy(&afterArray[0], &sourceArray[0], sizeof(sourceArray));
    copy(array.begin() + startIndex, array.begin() + endIndex, array.begin() + targetIndex);
    //copy(afterArray.begin(), afterArray.end(), back_inserter(beforeArray));

    return array;
}

// Binary Reader for vector<uint8_t>
class BinaryReader {
private:
    vector<uint8_t> ary;
    int curPos;
public:
    BinaryReader(vector<uint8_t> &array);
    int readByte();
    int readShortLE();
    int readIntLE();
    vector<uint8_t>& copyBytes(vector<uint8_t> &dst, int &offset, int &size);
    void seekAbs(int pos);
    void seekRel(int diff);
    int getPos();
};

BinaryReader::BinaryReader(vector<uint8_t> &array) {
    ary = array;
    curPos = 0;
}
int BinaryReader::readByte() {
    curPos ++;
    return ary[curPos - 1];
}
int BinaryReader::readShortLE() {
    curPos += 2;
    return ary[curPos - 2] + (ary[curPos - 1] << 8);
}
int BinaryReader::readIntLE() {
    curPos += 4;
    return ary[curPos - 4] + (ary[curPos - 3] << 8) + (ary[curPos - 2] << 16) + (ary[curPos - 1] << 24);
}
vector<uint8_t>& BinaryReader::copyBytes(vector<uint8_t> &dst, int &offset, int &size) {
    curPos += size;
    copy(ary.begin() + curPos - size, ary.begin() + curPos, dst.begin() + offset);
    return dst;
}
void BinaryReader::seekAbs(int pos) {
    curPos = pos;
}
void BinaryReader::seekRel(int diff) {
    curPos += diff;
}
int BinaryReader::getPos() {
    return curPos;
}

// Unity LZ4 Decompressor for vector<uint8_t>
class LZ4Decompressor {
public:
    /*LZ4Decompressor(vector<uint8_t> &array) {
        reader = new BinaryReader(array);
    }*/
    vector<uint8_t> decompress(vector<uint8_t> &array);
    int readAdditionalSize(BinaryReader &reader);
};

vector<uint8_t> LZ4Decompressor::decompress(vector<uint8_t> &array) {
    BinaryReader r(array);
    vector<uint8_t> retArray;
    int dataSize = 0;
    int decompressedSize = 0;

    int token = 0;
    int sqSize = 0;
    int matchSize = 0;
    int litPos = 0;
    int offset = 0;
    int retCurPos = 0;
    int endPos = 0;

    r.seekAbs(4);
    decompressedSize = r.readIntLE();
    dataSize = r.readIntLE();
    endPos = dataSize + 16;
    retArray = vector<uint8_t>(decompressedSize);

    r.seekAbs(16);

    // Start reading sequences
    while(true) {
        // Read the LiteralSize and MatchSize
        token = r.readByte();
        sqSize = token >> 4;
        matchSize = (token & 0x0f) + 4;
        if (sqSize == 15) {
            sqSize += readAdditionalSize(r);
        }

        // Copy the literal
        retArray = r.copyBytes(retArray, retCurPos, sqSize);
        retCurPos += sqSize;

        if (r.getPos() >= endPos - 1) {
            break;
        }

        // Read the offset
        offset = r.readShortLE();

        // Read the additional MatchSize
        if (matchSize == 19) {
            matchSize += readAdditionalSize(r);
        }

        // Copy the match properly
        if (matchSize > offset) {
            int matchPos = retCurPos - offset;
            while(true) {
                //retArray = copyWithin(retArray, retCurPos, matchPos, matchPos + offset);
                copy(retArray.begin() + matchPos, retArray.begin() + matchPos + offset, retArray.begin() + retCurPos);
                retCurPos += offset;
                matchSize -= offset;
                if (matchSize < offset) {
                    break;
                }
            }
        }
        //retArray = copyWithin(retArray, retCurPos, retCurPos - offset, retCurPos - offset + matchSize);
        copy(retArray.begin() + retCurPos - offset, retArray.begin() + retCurPos - offset + matchSize, retArray.begin() + retCurPos);
        retCurPos += matchSize;
    }
    return retArray;
}

int LZ4Decompressor::readAdditionalSize(BinaryReader &reader) {
    uint8_t size = reader.readByte();
    if (size == 255) {
        return size + readAdditionalSize(reader);
    } else {
        return size;
    }
}

std::vector<uint8_t> readFile(const char* filename)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    std::vector<uint8_t> vec;
    vec.reserve(fileSize);

    // read the data:
    vec.insert(vec.begin(),
               std::istream_iterator<uint8_t>(file),
               std::istream_iterator<uint8_t>());

    return vec;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " /path/to/file" << endl;
        return 1;
    } else {
        string filePath = string(argv[1]);

        LZ4Decompressor lz4;
        vector<uint8_t> vec = readFile(filePath);
        vector<uint8_t> outBuffer = lz4.decompress(vec);

        ofstream output(infile + ".dec", ios::binary);
        output.write((char *)&outBuffer[0], outBuffer.size());
    }
    return 0;
}
