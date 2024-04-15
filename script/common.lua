lualib={}

lualib.split=function(f,s)
	local a={}
	for w in string.gmatch(f, '[^'.. s ..']+') do
		a[#a+1]=w
	end
	return a
end





