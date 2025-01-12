

# drive_system.h

When accessing the file system through Toy (such as with the runner library), it's best practice to utilize the drive system - this system (tries to) prevent malicious accessing of files outside the designated folders. It does this by causing an error when a script tries to access a parent directory.

To use the drive system, first you must designate specific folders which can be accessed, like so:

```c
#include "drive_system.h"

int main(int argc, char* argv[]) {
    //the drive system uses a LiteralDictionary, which must be initialized with this
    Toy_initDriveSystem();

    Toy_setDrivePath("scripts", "assets/scripts");
    Toy_setDrivePath("sprites", "assets/sprites");
    Toy_setDrivePath("fonts", "assets/fonts");

    //TODO: do you stuff here

    //clean up the drive dictionary when you're done
    Toy_freeDriveSystem();

    return 0;
}
```

This utility is intended mainly for libraries to use - as such, the core of Toy does not utilize it.

### Implementation Details

The drive system uses a Toy's Dictionary structure to store the mappings between keys and values - this dictionary object is a static global which persists for the lifetime of the program.

## Defined Functions

### void Toy_initDriveSystem()

This function initializes the drive system.

### void Toy_freeDriveSystem()

This function cleans up after the drive system is no longer needed.

### void Toy_setDrivePath(char* drive, char* path)

This function sets a key-value pair in the drive system. It uses C strings, since it is intended to be called directly from `main()`.

### Toy_Literal Toy_getDrivePathLiteral(Toy_Interpreter* interpreter, Toy_Literal* drivePathLiteral)

This function, when given a string literal of the correct format, will return a new string literal containing the relative filepath to a specified file.

The correct format is `drive:/path/to/filename`, where `drive` is a drive that was specified with `Toy_setDrivePath()`.

On failure, this function returns a null literal.
