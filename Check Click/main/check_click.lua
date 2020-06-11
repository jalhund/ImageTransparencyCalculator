local M = {}

local data = {}
local data_compression = {}

--Modified: https://github.com/dapetcu21/crit/blob/master/crit/pick.lua
local h_size = hash("size")

local function pick_sprite(game_object, x, y)
	local url = msg.url(nil, game_object, "sprite")
	local size = go.get(url, h_size)
  	local transform = go.get_world_transform(url)
	local pos = vmath.inv(transform) * vmath.vector4(x, y, 0, 1)
	x, y = pos.x, pos.y
	
 	local half_width = size.x * 0.5
	local left = -half_width
	local right = half_width
	if x < left or x > right then return false end
	
 	local half_height = size.y * 0.5
	local top = half_height
	local bottom = -half_height
	if y < bottom or y > top then return false end
	
 	return true
end

local function check_transparency(game_object, x, y)
	local url = msg.url(nil, game_object, "sprite")
	local size = go.get(url, h_size)
  	local transform = go.get_world_transform(url)
	local pos = vmath.inv(transform) * vmath.vector4(x, y, 0, 1)
	x, y = pos.x, pos.y
	
	local compression = data_compression[game_object]

 	x = math.floor(x + size.x * 0.5) + 1
	y = math.floor(y + size.y * 0.5) + 1
	local i = math.floor((size.y-y)/compression)*math.floor(size.x/compression) + math.floor(x/compression + 0.5)
	print("Now i is:", i, math.floor(x/compression + 0.5), math.floor((size.y-y)/compression))

	return data[game_object][i] == 255
end

function M.init(init_list)
	for k, v in pairs(init_list) do
		local file = io.open("generated_data/"..v..".data", "rb")
  		if file then
  			data[v] = {}
  			data_compression[v] = file:read("*number")
  			while true do
  				local val = file:read("*number")
 		 		if val == nil then
 		 			break
 		 		end
  				table.insert(data[v], val)
			end
  		else
  			print("File not opened: ", v)
  		end
	end
	pprint("data:", data)
end

function M.check(game_object, x, y)
	print("check: ", x, y)
	if pick_sprite(game_object, x, y) and check_transparency(game_object, x, y) then
		return true
	else
		return false
	end
end

return M