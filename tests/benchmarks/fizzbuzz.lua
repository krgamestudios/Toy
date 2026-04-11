for counter = 1, 10000 do
	local result = ""

	if counter % 3 == 0 then
		result = result .. "fizz"
	end

	if counter % 5 == 0 then
		result = result .. "buzz"
	end

	if result ~= "" then
		print(result)
	else
		print(counter)
	end
end
