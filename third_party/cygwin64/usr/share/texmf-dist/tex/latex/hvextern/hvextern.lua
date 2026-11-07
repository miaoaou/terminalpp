-----------------------------------------------------------------------
--         FILE:  hvextern.lua
--  DESCRIPTION:  check two files for identical code
-- REQUIREMENTS:  
--       AUTHOR:  Herbert Voß
--      LICENSE:  LPPL1.3
--
-- $Id: hvextern.lua 578 2022-06-20 16:41:45Z herbert $
-----------------------------------------------------------------------
kpse.set_program_name("luatex")

function GetFileContents(f1,f2)
	local NewContents, OldContents
	local File = io.open(f1)
	if File then
		NewContents = File:read('*all')
		File:close()
	end
	local File = io.open(f2)
	if File then
		OldContents = File:read('*all')
		File:close()
	end
	return NewContents == OldContents
end

--local f1 = arg[1]
--local f2 = arg[2]