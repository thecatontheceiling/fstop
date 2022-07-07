# F-STOP

## Code Guidelines

To log messages to console, use our custom orange colored source2-style logging channel:

```cpp
Log_Msg( LOG_FSTOP, "Message String: %s\n", param );
```

Use `#ifdef GAME_DLL` and `#ifdef CLIENT_DLL` to distinguish between client/server. For these macros, avoid using `#ifndef` because the single 'n' is hard to notice so it can easily create confusion about what code is on client or server.

### Default Source Files

`fstop_example.cpp`:
```cpp
//==== Copyright © 2017-2019, Lever Softworks, All rights reserved. ====//
//
// Purpose: F-STOP C++ File
//
//======================================================================//

#include "cbase.h"
#include "fstop_example.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
```

Change **Purpose:** to a description of the specific file.

`fstop_example.h`:
```cpp
//==== Copyright © 2017-2019, Lever Softworks, All rights reserved. ====//
//
// Purpose: F-STOP C++ Header File
//
//======================================================================//

#ifndef FSTOP_EXAMPLE_H
#define FSTOP_EXAMPLE_H
#ifdef _WIN32
#pragma once
#endif


#endif // FSTOP_EXAMPLE_H
```

## Convars

* Don't define/construct ConVar objects more than once in the code unless it is otherwise unavoidable.
* Try to declare ConVars only once, ideally in shared headers like `fstop_common.h`
* Use `snake_case` only for cvar names. Put an underscore between every word.
* Only client-only variables may start with `cl_`
* Replicated or server-only vars may start with `sv_`

## Fast Path Rendering

To disable Fast Path rendering for your client entity add the following to the class definition:

```cpp
virtual IClientModelRenderable*	GetClientModelRenderable() { return NULL; } // disable fast path
```
