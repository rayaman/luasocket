# LuaSocket-Lanes

This is a fork of LuaSocket library (v3.0-rc2)

Original Repo at above version can be found [here](https://github.com/diegonehab/luasocket/tree/22cd5833fcc0e272f26004a79c8545e959ba406b)

It provides the ability to use LuaSocket with the Lua Lanes library, allowing 
the server listener to be on one thread and the client handlers to be on seperate threads.

## Quick Start

Install the module using luarocks (get luarocks [here](https://github.com/luarocks/luarocks/wiki/Download))

```shell
luarocks install luasocket-lanes
```

On a server thread you can use the following logic to process client connections in a seperate thread:

```lua
local lanes = require "lanes".configure
local luaSocket = require "socket-lanes"

local server, err = luaSocket.bind("localhost", 8888)

while true do
    local clientSocketFileDescriptor, err = server:acceptfd()

    -- handle client request in seperate thread
    local clientThread, err = lanes.gen(function(fd)
        local luaSocket = require "socket-lanes"
        
        local client, err = luaSocket.tcp(fd)

        -- do stuff with client...

        client:close()
    end)(clientSocketFileDescriptor)
end
```

*I have ommited error handling from the above example, all the `err` variables should be checked before the next operation.*

**Note: All luasocket modules are provided by luasocket-lanes, but with the postfix '-lanes', this is to allow side by side install of luasocket and luasocket-lanes.**

----

## Other enhancements

A few more socket options are now supported. You can now get/set the following using the `socket:getoption` and `socket:setoption` methods:

- `rcvbuf` - `int` - **The recieve buffer size in bytes** (`SO_RCVBUF`)
- `sndbuf` - `int` - **The send buffer size in bytes** (`SO_SNDBUF`)
- `debug` - `bool` - **Flag for socket debugging** (`SO_DEBUG`)

All of the above options are documented in more detail [here](http://man7.org/linux/man-pages/man7/socket.7.html) (*search for the option code in brackets above*)

---

## Note about Lua Lanes

While this library was made for working with Lua Lanes, it does not require that library to be installed and used.
Because file descriptors are simple numbers, you could use this to deal with client requests anywhere you wish, using 
another lua threading library, other language runtimes etc.

----

## LuaSocket

This is the LuaSocket 3.0-rc1. It has been tested on Windows 7, Mac OS X,
and Linux. 

Please use the project page at GitHub 

    https://github.com/diegonehab/luasocket

to file bug reports or propose changes. 

Have fun,
Diego Nehab.
