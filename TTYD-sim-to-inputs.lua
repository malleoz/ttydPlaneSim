local lines = nil
local delimited = {}
local inputs = {}

function onScriptStart()
	startFrame = GetFrameCount()
	for line in io.lines("simRes.txt") do
		lines = line .. ','
	end
	
	-- Put comma-delimited values into a table
	for x in lines:gmatch("(.-),") do
			table.insert(delimited, x)
	end
	
	-- Skip over the first 10 values in the table
	-- Then, every fifth value is the input
	for x=11,#delimited,5 do
		table.insert(inputs, delimited[x])
	end
	
	return
end

function onScriptCancel()
end

function stickToInput(stick)
	-- This function assumes unsigned stick values because I am lazy
	-- 184 to 255 is left (to center)
	-- 1 to 72 is right
	input = 128
	if stick < 0 then
		input = input - 15 + stick
	elseif stick > 0 then
		input = input + 15 + stick
	end
	return input
end

function onScriptUpdate()
	curFrame = GetFrameCount() - startFrame
	if curFrame+1 > #inputs then
		MsgBox("Collided.")
		CancelScript()
		return
	end
	stick = tonumber(inputs[curFrame+1])
	input = stickToInput(stick)
	SetMainStickX(input)
end

function onStateLoaded()
end
