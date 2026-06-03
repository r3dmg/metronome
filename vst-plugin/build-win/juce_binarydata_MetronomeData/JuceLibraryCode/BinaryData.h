/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   kick_wav;
    const int            kick_wavSize = 66468;

    extern const char*   snare_wav;
    const int            snare_wavSize = 88920;

    extern const char*   hihat_closed_wav;
    const int            hihat_closed_wavSize = 10230;

    extern const char*   hihat_open_wav;
    const int            hihat_open_wavSize = 112420;

    extern const char*   crash_wav;
    const int            crash_wavSize = 208544;

    extern const char*   tom_wav;
    const int            tom_wavSize = 173384;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 6;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
