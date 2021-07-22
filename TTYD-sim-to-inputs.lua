local lines = nil
local delimited = {}
local inputs = {}

function onScriptStart()
	startFrame = GetFrameCount()
	for line in io.lines("simRes.txt") do
		lines = line .. ','
		text = ''
		for x in lines:gmatch("(.-),") do
			table.insert(delimited, x)
		end
		
		-- Every fifth value is the input
		for x=6,#delimited,5 do
			table.insert(inputs, delimited[x])
		end
		
		return
	end
end

function onScriptCancel()
end

function stickToInput(stick)
	-- This function assumes unsigned stick values because I am lazy
	-- 184 to 255 is left (to center)
	-- 1 to 72 is right
	if stick >= 184 then
		return stick - 143
	else
		return stick + 143
	end
end

function onScriptUpdate()
	curFrame = GetFrameCount() - startFrame + 1
	MsgBox(curFrame)
	stick = tonumber(inputs[curFrame])
	input = stickToInput(stick)
	SetMainStickX(input)
end

function onStateLoaded()
end
