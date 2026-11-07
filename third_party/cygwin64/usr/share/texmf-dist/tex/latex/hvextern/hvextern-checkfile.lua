#!/usr/bin/env texlua
-----------------------------------------------------------------------
--         FILE:  hvextern-checkfile.lua
--  DESCRIPTION:  check two files for identical code
-- REQUIREMENTS:  
--       AUTHOR:  Herbert Voß
--      LICENSE:  LPPL1.3
--
-- $Id: hvextern-checkfile.lua 585 2022-07-21 13:28:06Z herbert $
-----------------------------------------------------------------------
kpse.set_program_name("luatex")

local f = kpse.find_file("hvextern.lua")
require('hvextern')

--[[
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
]]

local f1 = arg[1]
local f2 = arg[2]
diffFile = io.open(f1..".diff","w+")
if GetFileContents(f1,f2) then
    diffFile:write("\\let\\hvCheckNewFile\\empty")
else
    diffFile:write("\\def\\hvCheckNewFile{Code changes}")
end
diffFile:close()

