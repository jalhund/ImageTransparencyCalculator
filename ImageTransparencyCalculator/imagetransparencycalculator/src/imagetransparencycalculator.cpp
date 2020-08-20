#define LIB_NAME "ImageTransparencyCalculator"
#define MODULE_NAME "imagetransparencycalculator"

// include the Defold SDK
#include <dmsdk/sdk.h>
#include <iostream>

static int calctransparency(lua_State* L)
{

    dmScript::LuaHBuffer *lua_buffer = dmScript::CheckBuffer(L, 1);
    dmBuffer::HBuffer buffer = lua_buffer->m_Buffer;
    int width, height, channels;
    uint8_t *bytes;
    uint32_t src_size;
    
    if (!dmBuffer::IsBufferValid(buffer))
    {
        luaL_error(L, "Buffer is invalid");
    }
    dmBuffer::Result result = dmBuffer::GetBytes(buffer, (void **)&bytes, &src_size);
    if (result != dmBuffer::RESULT_OK)
    {
        luaL_error(L, "Buffer is invalid");
    }
    uint8_t *copied_bytes = (uint8_t*)malloc(src_size);

    for(int i = 0; i < src_size;++i)
    {
        copied_bytes[i] = bytes[i];
    }

    width = lua_tointeger(L, 2);
    if (width == 0)
    {
        luaL_error(L, "'width' of the buffer should be an integer and > 0");
    }
    
    height = lua_tointeger(L, 3);
    if (height == 0)
    {
        luaL_error(L, "'height' of the buffer should be an integer and > 0");
    }
    
    channels = lua_tointeger(L, 4);
    if (channels == 0)
    {
        luaL_error(L, "'channels' should be an integer and 3 or 4");
    }

    const dmBuffer::StreamDeclaration streams_decl[] = 
    {
        {dmHashString64("byte"), dmBuffer::VALUE_TYPE_UINT8, 1},
    };

    dmBuffer::Destroy(buffer);

    dmBuffer::Result r_create = dmBuffer::Create(src_size/4, streams_decl, 1, &buffer);
    if (r_create == dmBuffer::RESULT_OK) 
    {
        uint8_t* out_bytes;
        uint32_t out_size;
        dmBuffer::Result r_get_out_bytes = dmBuffer::GetBytes(buffer, (void**)&out_bytes, &out_size);
        if (r_get_out_bytes == dmBuffer::RESULT_OK) 
        {
            for(int i = 3; i < src_size; i+=4)
            {
                out_bytes[i/4] = copied_bytes[i];
            }
            lua_buffer->m_Buffer = buffer;
        }
        else 
        {
            luaL_error(L, "Invalid get new bytes");
        }
    } 
    else 
    {
        luaL_error(L, "Invalid create new buffer");
    }
    free(copied_bytes);
    return 1;
}

static int resize(lua_State* L)
{

    dmScript::LuaHBuffer *lua_buffer = dmScript::CheckBuffer(L, 1);
    dmBuffer::HBuffer buffer = lua_buffer->m_Buffer;
    int width, height, compression;
    uint8_t *bytes;
    uint32_t src_size;
    
    if (!dmBuffer::IsBufferValid(buffer))
    {
        luaL_error(L, "Buffer is invalid");
    }
    dmBuffer::Result result = dmBuffer::GetBytes(buffer, (void **)&bytes, &src_size);
    if (result != dmBuffer::RESULT_OK)
    {
        luaL_error(L, "Buffer is invalid");
    }
    uint8_t *copied_bytes = (uint8_t*)malloc(src_size);
    
    for(int i = 0; i < src_size;++i)
    {
        copied_bytes[i] = bytes[i];
    }

    width = lua_tointeger(L, 2);
    if (width == 0)
    {
        luaL_error(L, "'width' of the buffer should be an integer and > 0");
    }
    
    height = lua_tointeger(L, 3);
    if (height == 0)
    {
        luaL_error(L, "'height' of the buffer should be an integer and > 0");
    }
    
    compression = lua_tointeger(L, 4);
    if (compression < 1)
    {
        luaL_error(L, "'compression' should be 1 or more");
    }

    const dmBuffer::StreamDeclaration streams_decl[] = 
    {
        {dmHashString64("byte"), dmBuffer::VALUE_TYPE_UINT8, 1},
    };

    dmBuffer::Destroy(buffer);

    dmBuffer::Result r_create = dmBuffer::Create(src_size/compression/compression, streams_decl, 1, &buffer);
    if (r_create == dmBuffer::RESULT_OK) 
    {
        uint8_t* out_bytes;
        uint32_t out_size;
        dmBuffer::Result r_get_out_bytes = dmBuffer::GetBytes(buffer, (void**)&out_bytes, &out_size);
        if (r_get_out_bytes == dmBuffer::RESULT_OK) 
        {
            double xscale = 1.0/compression;
            double yscale = 1.0/compression;
            double threshold = 0.5 / (xscale * yscale);
            double yend = 0.0;
            for (int f = 0; f < height/compression; f++) // y on output
            {
                double ystart = yend;
                yend = (f + 1) / yscale;
                if (yend >= height) yend = height - 0.000001;
                double xend = 0.0;
                for (int g = 0; g < width/compression; g++) // x on output
                {
                    double xstart = xend;
                    xend = (g + 1) / xscale;
                    if (xend >= width) xend = width - 0.000001;
                    double sum = 0.0;
                    for (int y = (int)ystart; y <= (int)yend; ++y)
                    {
                        double yportion = 1.0;
                        if (y == (int)ystart) yportion -= ystart - y;
                        if (y == (int)yend) yportion -= y+1 - yend;
                        for (int x = (int)xstart; x <= (int)xend; ++x)
                        {
                            double xportion = 1.0;
                            if (x == (int)xstart) xportion -= xstart - x;
                            if (x == (int)xend) xportion -= x+1 - xend;
                            sum += copied_bytes[y*width + x] * yportion * xportion;
                        }
                    }
                    out_bytes[f*width/compression + g] = (sum > threshold) ? 1 : 0;
                }
            }
            lua_buffer->m_Buffer = buffer;
        }
        else 
        {
            luaL_error(L, "Invalid get wav bytes");
        }
    } 
    else 
    {
        luaL_error(L, "Invalid create new buffer");
    }
    free(copied_bytes);
    return 1;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
    {"calctransparency", calctransparency},
    {"resize", resize},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeImageTransparencyCalculator(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeImageTransparencyCalculator(dmExtension::Params* params)
{
    // Init Lua
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeImageTransparencyCalculator(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeImageTransparencyCalculator(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}


// Defold SDK uses a macro for setting up extension entry points:
//
// DM_DECLARE_EXTENSION(symbol, name, app_init, app_final, init, update, on_event, final)

// ImageTransparencyCalculator is the C++ symbol that holds all relevant extension data.
// It must match the name field in the `ext.manifest`
DM_DECLARE_EXTENSION(ImageTransparencyCalculator, LIB_NAME, AppInitializeImageTransparencyCalculator, AppFinalizeImageTransparencyCalculator, InitializeImageTransparencyCalculator, 0, 0, FinalizeImageTransparencyCalculator)