local image_data = require "main.image_data"

local M = {}

local function calc(image_id, compression)
  	local width = image_data[image_id].x
  	local height = image_data[image_id].y
  	local channels = 4

  	local file = io.open("images/"..image_id..".png", "rb")
  	local bytes = file:read("*a")
  	file:close()
  	local png = image.load(bytes,true)
  	local png_buffer = png.buffer
  	print("loaded size: ", png.width, png.height, png.type, #png_buffer)
  	local generation_buffer = buffer.create(#png_buffer, { {name=hash("byte"), type=buffer.VALUE_TYPE_UINT8, count=1} } )
  	local stream = buffer.get_stream(generation_buffer, hash("byte"))
  	for i = 1, #png_buffer do
  	  	stream[i] = string.byte(png_buffer, i)
  	end
  	imagetransparencycalculator.calctransparency(generation_buffer, width, height, channels)
  	imagetransparencycalculator.resize(generation_buffer, width, height, compression)
  	file = io.open("generated_data/"..image_id..".data", "wb")
  	stream = buffer.get_stream(generation_buffer, hash("byte"))
  	local text_file = compression.." "
    for i = 1, width/compression*height/compression do
    	print(type(stream[i]), stream[i])
     	text_file = text_file..stream[i].." "
    end
    text_file = zlib.deflate(text_file)
    file:write(text_file)
    file:close()
  	--print("clicked: ", stream[math.floor(y)*self.buffer_info.width + math.floor(x)])
end

function M.handle(compression)
	for k, v in pairs(image_data) do
		calc(k, compression)
	end
	os.exit()
end

return M