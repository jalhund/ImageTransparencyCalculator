local M = {}

local count = 2 --Counts needed
local time = 0.4
local start_time
local pressed_count = 0
local callback

function M.set_callback(callback_)
	callback = callback_
end

function M.on_input(self, action_id, action)
	if action.released and action_id == hash("touch") then
		if pressed_count == 0 then
			timer.delay(time, false, function()
				pressed_count = 0
			end)
		end
		pressed_count = pressed_count + 1
		if pressed_count == count then
			callback()
		end
	end
end

	
return M