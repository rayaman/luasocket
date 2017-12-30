# LuaSocket-Lanes

This is a fork of LuaSocket library (v3.0-rc2)

Original Repo at above version can be found [here](https://github.com/diegonehab/luasocket/tree/22cd5833fcc0e272f26004a79c8545e959ba406b)

It provides the ability to use LuaSocket with the Lua Lanes library, allowing 
the server listener to be one thread and the client handlers to be on seperate
threads.

## Quick Start

Install the module using luarocks (get luarocks [here](https://github.com/luarocks/luarocks/wiki/Download))

```shell
luarocks install luasocket-lanes
```

*For now the original LuaSocket and LuaSocket-Lanes cannot co-exist in the same lua paths*

On a server thread you can use the following logic to process client connections in seperate thread:

```lua
local lanes = require "lanes".configure
local luaSocket = require "socket"

local server, err = luaSocket.bind("localhost", 8888)

while true do
    local clientSocketFileDescriptor, err = server:acceptfd()

    -- handle client request in seperate thread
    local clientThread, err = lanes.gen(function(fd)
        local luaSocket = require "socket"
        
        local client, err = luaSocket.tcp(fd)

        -- do stuff with client...

        client:close()
    end)(clientSocketFileDescriptor)
end
```

*I have ommited error handling from the above example, all the ```err``` variables should be checked before the next operation*

----

## LuaSocket

This is the LuaSocket 3.0-rc1. It has been tested on Windows 7, Mac OS X,
and Linux. 

Please use the project page at GitHub 

    https://github.com/djfdyuruiry/luasocket-lanes

to file bug reports or propose changes. 

Have fun,
Diego Nehab.
