function debug ()
	print("Swaps x and y");
end

function process()
	x, y = getpos()
	setpos(y, x)
end