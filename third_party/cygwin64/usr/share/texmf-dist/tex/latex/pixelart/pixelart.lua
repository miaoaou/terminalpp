--[[
   Copyright 2017-2023 Louis Paternault

   This work may be distributed and/or modified under the
   conditions of the LaTeX Project Public License, either version 1.3
   of this license or (at your option) any later version.
   The latest version of this license is in
     http://www.latex-project.org/lppl.txt
   and version 1.3 or later is part of all distributions of LaTeX
   version 2005/12/01 or later.

   This work has the LPPL maintenance status `maintained'.

   The Current Maintainer of this work is Louis Paternault

   This work consists of the files pixelart.sty, pixelart.lua, pixelart.tex.
--]]

require("lualibs-lpeg")
local luakeys = require("luakeys")()

pixelart = {
  _debug = false,
  _counter = 0,
}

--------------------------------------------------------------------------------
--[[ Debug on/off
--]]

local function pixelart_setpixelartdebug(flag)
  luakeys.opts.debug = flag
  pixelart._debug = flag
end

pixelart.setpixelartdebug = pixelart_setpixelartdebug

--------------------------------------------------------------------------------
--[[ Print
--]]

local function tex_sprint(text)
    tex.sprint(text)
    if pixelart._debug then
        io.write(text)
    end
end

local function tex_print(text)
    tex.print(text)
    if pixelart._debug then
        io.write(text, "\n")
    end
end

--------------------------------------------------------------------------------
--[[ Define and use colors
-- ]]
pixelart._colors = {}

local function pixelart_parsecolors(argument)
  return luakeys.parse(
      argument,
      {
        naked_as_value = true,
        hooks = {
          keys = function(key, value, depth, current, result)
            return tostring(key), value
          end,
        }
      }
    )
end

local function pixelart_newpixelartcolors(name, argument)
  if pixelart._colors[name] == nil then
    pixelart._colors[name] = pixelart_parsecolors(argument)
  else
    error(string.format("Error: Colors '%s' is already defined.", name))
  end
end

local function pixelart_renewpixelartcolors(name, argument)
  pixelart._colors[name] = pixelart_parsecolors(argument)
end

pixelart.newpixelartcolors = pixelart_newpixelartcolors
pixelart.renewpixelartcolors = pixelart_renewpixelartcolors

-- Default color sets
pixelart._colors["explicit"] = {}

pixelart._colors["RGB"] = {
  R = "red",
  G = "green",
  B = "blue",
  W = "white",
  K = "black"
}

pixelart._colors["BW"] = {
  ["0"] = "white",
  ["1"] = "black",
}

pixelart._colors["gray"] = {
  ["0"] = "white",
  ["1"] = "white!89!black",
  ["2"] = "white!78!black",
  ["3"] = "white!67!black",
  ["4"] = "white!56!black",
  ["5"] = "white!44!black",
  ["6"] = "white!33!black",
  ["7"] = "white!22!black",
  ["8"] = "white!11!black",
  ["9"] = "black",
}

pixelart._colors["mono"] = {}
for _, char in pairs(string.explode("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", "")) do
  pixelart._colors["mono"][char] = ""
end


--------------------------------------------------------------------------------
--[[ Options
--]]

local ALGORITHMS = {squares = true, stack = true}

local function parse(str, default)
  local parser = luakeys.define({
    tikz = {},
    colors = {
      process = function(value, input, result, unknown)
        -- If argument is a string, convert numeric keys into strings
        if type(value) ~= "table" then
          return value
        end

        local converted = {}
        for k, v in pairs(value) do
          converted[tostring(k)] = v
        end
        return converted
      end,
    },
    margin = {
      data_type = "number",
      default = 0,
      always_present = true,
    },
    style = {
      data_type = "string",
      default = "pixelart",
      always_present = true,
    },
    squares = {
      exclusive_group = "algo",
    },
    stack = {
      exclusive_group = "algo",
    },
    draft = {
      data_type = "boolean",
    },
  })

  options = parser(str, {
    defaults = default,
  })

  -- Algo option
  local algo = nil
  for key, value in pairs(ALGORITHMS) do
    if options[key] then
      algo = key
    end
  end
  if not algo then
    options[pixelart._default_algo[1]] = pixelart._default_algo[2]
  end

  -- Convert Tikz option back to string
  if type(options.tikz) == type({}) then
    options.tikz = luakeys.render(options.tikz)
  end

  return options
end

local DEFAULT = {colors = "mono"}
local function pixelart_setpixelartdefault(str)
   pixelart._default = parse(str, DEFAULT)

   for key, value in pairs(pixelart._default) do
     if ALGORITHMS[key] then
       pixelart._default_algo = {key, value}
       pixelart._default[key] = nil
     end
   end
end

pixelart._default = {colors = "mono"}
pixelart._default_algo = {"stack", {}}
pixelart.setpixelartdefault = pixelart_setpixelartdefault

--------------------------------------------------------------------------------
--[[ Parse pixelart string
--Parse arguments, and build a table of tables.
--]]
--
local lineRE = lpeg.Ct(
  -- Regular expression to match a line of colors
  (
    (lpeg.P("{") * lpeg.C((lpeg.P(1) - lpeg.S("{}"))^0) * lpeg.P("}"))
    +
    lpeg.C((lpeg.P(1) - lpeg.S("{}")))
  )^0
)

local function str2arrays(str)
  -- Turn the \pixelart{} argument into a table (of lines) of tables (of colors).

  -- Trim string: remove leading and trailing whitespaces
  str = str:gsub("^%s*(.-)%s*$", "%1")

  -- Turn it into a table (which is called "array" not to clash with the "table" library)
  local array = {}
  for k, line in ipairs(string.explode(str, " ")) do
    array[k] = lineRE:match(line)
  end

  -- Ensure each row has the same number of columns
  local length = 0
  for k, line in ipairs(array) do
    if #line > length then
      length = #line
    end
  end
  for k, line in ipairs(array) do
    if #line < length then
      for i=#line+1, length do
        line[i] = false
      end
    end
  end

  -- Flip array so that flipped[x][y] gives the pixel with coordinates (x, y), in the standard frame
  local flipped = {}
  for y, line in ipairs(array) do
    for x, color in ipairs(array[y]) do
      if y == 1 then
        flipped[x] = {}
      end
      flipped[x][#array - y + 1] = color
    end
  end

  return flipped
end

--------------------------------------------------------------------------------
--[[ Color tools
--]]
local function color2color(colors, color)
  -- Convert a color (as given by the user) into a color (usable by TikZ).
  if color == "." or not color then
    -- Transparent pixel: do not print anything
    return nil
  elseif colors[color] and colors[color] ~= "" then
    -- A color has been defined: use it
    return string.format("color=%s", colors[color])
  elseif colors[color] and colors[color] == "" then
    -- An empty color has been defined: use the default TikZ color
    return ""
  else
    -- No color has been defined: use the argument as the TikZ color
    return color
  end
end

--------------------------------------------------------------------------------
-- Turn pixelart string into TikZ code, using the SQUARES algorithm

local function pixelart_body_squares(array, colors, options)
  -- Draw the tikz pixels, as a set of squares.

  if #array == 0 then
    -- Empty array
    return
  end

  tex_print(string.format(
    [[\clip ({0-%s}, {0-%s}) rectangle (%s, %s); ]],
    options.margin,
    options.margin,
    #array + options.margin,
    #array[1] + options.margin
    ))

  for x, column in ipairs(array) do
    for y, color in ipairs(column) do
      color = color2color(colors, color)

      ---------------------
      -- Which pixel size?
      local overlap
      if type(options.squares) == type({}) then
        overlap = options.squares["overlap"] or "0"
      else
        overlap = "0"
      end

      ---------------------
      -- At last, we can display the pixel…
      if color ~= nil then
        tex_print(string.format([[\fill[%s, %s] (%s, %s) rectangle ++(1+%s, 1+%s);]],
          options.style,
          color,
          string.format("{%s-%s}", x-1, overlap),
          string.format("{%s-%s}", y-1, overlap),
          2*overlap,
          2*overlap
        ))
      end
    end
  end
end

--------------------------------------------------------------------------------
-- Turn pixelart string into TikZ code, using the STACK algorithm

local function remove_zone(write, coord, read)
  -- Remove the zone of the pixel `coord`
  -- If both write and read are present (they are expected to be 2D-arrays of the same size, then read zones from `read`, and remove them from `write`.

  -- Default values for options
  if read == nil then
    read = write
  end
  if colorblind == nil then
    colorblind = false
  end

  local originalcolor = read[coord[1]][coord[2]]
  local samecolor = function(color) return color == originalcolor end

  -- Go!
  local stack = {}
  table.insert(stack, coord)
  while #stack ~= 0 do
    local current = table.remove(stack)
    for _, neighbour in pairs({
      {current[1]-1, current[2]},
      {current[1]+1, current[2]},
      {current[1], current[2]-1},
      {current[1], current[2]+1},
      }) do
      if (
        neighbour[1] >= 1 and neighbour[1] <= #read -- First coordinate inside the array
        and
        neighbour[2] >= 1 and neighbour[2] <= #read[1] -- Second coordinate inside the array
        and
        write[neighbour[1]][neighbour[2]] -- Not processed yet
        and
        samecolor(read[neighbour[1]][neighbour[2]]) -- Same color
        ) then
        table.insert(stack, neighbour)
      end
    end
    write[current[1]][current[2]] = false
  end
end

local border_transitions = {
  westtop = {
    tests = {
      {1, 1},
      {1, 0},
    },
    next = {
      ["true true"] = {
        step = {1, 1},
        state = "northleft",
        mark = {0, 0},
      },
      ["false true"] = {
        step = {1, 0},
        state = "westtop",
        mark = nil,
      },
      ["true false"] = {
        step = {0, 0},
        state = "southright",
        mark = {1, 1},
      },
      ["false false"] = {
        step = {0, 0},
        state = "southright",
        mark = {1, 1},
      },
    },
  },
  northleft = {
    tests = {
      {-1, 1},
      {0, 1},
    },
    next = {
      ["true true"] = {
        step = {-1, 1},
        state = "eastbottom",
        mark = {1, 0},
      },
      ["true false"] = {
        step = {0, 0},
        state = "westtop",
        mark = {0, 1},
      },
      ["false true"] = {
        step = {0, 1},
        state = "northleft",
        mark = nil,
      },
      ["false false"] = {
        step = {0, 0},
        state = "westtop",
        mark = {0, 1},
      },
    },
  },
  southright = {
    tests = {
      {0, -1},
      {1, -1},
    },
    next = {
      ["true true"] = {
        step = {1, -1},
        state = "westtop",
        mark = {0, 1},
      },
      ["false true"] = {
        step = {0, 0},
        state = "eastbottom",
        mark = {1, 0},
      },
      ["true false"] = {
        step = {0, -1},
        state = "southright",
        mark = nil,
      },
      ["false false"] = {
        step = {0, 0},
        state = "eastbottom",
        mark = {1, 0},
      },
    },
  },
  eastbottom = {
    tests = {
      {-1, 0},
      {-1, -1},
    },
    next = {
      ["true true"] = {
        step = {-1, -1},
        state = "southright",
        mark = {1, 1},
      },
      ["false true"] = {
        step = {0, 0},
        state = "northleft",
        mark = {0, 0},
      },
      ["true false"] = {
        step = {-1, 0},
        state = "eastbottom",
        mark = nil,
      },
      ["false false"] = {
        step = {0, 0},
        state = "northleft",
        mark = {0, 0},
      },
    },
  },
}

local function iter_border(array, start, colorblind)
  -- If colorblind==true, all colors are considered the same (that is, a pixel is either transparent or colored, with no difference between the colors)
  local samecolor
  if colorblind then
    samecolor = function(current, test)
      if (
        current[1] + test[1] < 1
        or
        current[1] + test[1] > #array
        ) then
        return false
      end
      color = array[current[1] + test[1]][current[2] + test[2]]
      if color == nil or color == false then
        return false
      end
      return (
        color == "." and array[start[1]][start[2]] == "."
        ) or (
        color ~= "." and array[start[1]][start[2]] ~= "."
        )
    end
  else
    samecolor = function(current, test)
      if (
        current[1] + test[1] < 1
        or
        current[1] + test[1] > #array
        ) then
        return false
      end
      color = array[current[1] + test[1]][current[2] + test[2]]
      return color == array[start[1]][start[2]]
    end
  end

  local current = {start[1], start[2]}
  local state = "northleft"

  return function()
    while true do
      tests = border_transitions[state].tests
      local transition = border_transitions[state].next[string.format(
        "%s %s",
        samecolor(current, tests[1]),
        samecolor(current, tests[2])
        )]
      state = transition.state
      current = {
        current[1] + transition.step[1],
        current[2] + transition.step[2]
      }
      if transition.mark then
        if current[1] == start[1] and current[2] == start[2] and state == "northleft" then
          return
        else
          return {
            current[1] + transition.mark[1],
            current[2] + transition.mark[2],
          }
        end
      end
    end
  end
end

local function iter_unprocessed_zones(array)
  -- Iterate coordinates of pixels that haven't been processed yet (their value is not false)
  local x = 1
  local y = 1

  return function()
    while not array[x][y] do
      x = x + 1
      if x > #array then
        x = 1
        y = y + 1
        if y > #array[1] then
          return
        end
      end
    end
    return {x, y}
  end
end

local function pixelart_body_stack(array, colors, options)
  -- The first argument is an array of lines, each of them being an array of colors (i.e. color of the first pixel of the line, color of the second pixel of the line, etc).
  -- Some "colors" have meaning:
  -- - false (the boolean): pixel has already been processed
  -- - "." (the character dot): pixel is transparent
  -- - anything else: the color of the pixel

  if #array == 0 then
    return
  end

  tex_print([[\begin{scope}[even odd rule] ]])

  -- Clip away transparent zones
  tex_print(string.format(
    [[\clip ({0-%s}, {0-%s}) rectangle (%s, %s) ]],
    options.margin,
    options.margin,
    #array + options.margin,
    #array[1] + options.margin
    ))

  for x = 1, #array do
    for y = 1, #array[x] do
      if array[x][y] == "." then
        tex_print(string.format("(%s, %s) rectangle ++(1, 1) ", x-1, y-1))
      end
    end
  end

  tex_print(";")

  -- Draw color zones
  for current in iter_unprocessed_zones(array) do
    local color = color2color(colors, array[current[1]][current[2]])
    if color == nil then
      -- Nothing to do: transparent zone
    else
      tex_sprint(string.format([[\fill[%s, %s] (%s, %s) ]], options.style, color, current[1]-1, current[2]-1))
      for coord in iter_border(array, current, false) do
        tex_sprint(string.format([[ -- (%s, %s) ]], coord[1]-1, coord[2]-1))
      end
      tex_print(string.format([[ -- cycle ;]]))
    end
    remove_zone(array, {current[1], current[2]})
  end

  tex_print([[\end{scope} ]])
end

--------------------------------------------------------------------------------
-- Functions pixelart() and tikzpixelart()

local function pixelart_body(str, options)
  -- Debug
  pixelart._counter = pixelart._counter + 1
  if pixelart._debug then
    io.write("\n%", str, "\n")
    io.write("% pixelart ", pixelart._counter, ", file ", status.filename, ", input line ", tex.inputlineno, "\n")
  else
    io.write("(pixelart ", pixelart._counter, ", file ", status.filename, ", input line ", tex.inputlineno)
  end

  -- Colors
  local colors
  if type(options.colors) == "table" then
    colors = options.colors
  else
    colors = pixelart._colors[options.colors]
  end

  if (pixelart._draft or options.draft) and options.draft ~= false then
    local array = str2arrays(str)
    tex_print(string.format(
      [[ \draw[pattern=checkerboard] (0, 0) rectangle (%s, %s); ]],
      #array, #array[1]
      ))
  elseif options.stack then
    pixelart_body_stack(str2arrays(str), colors, options)
  else -- options.squares is the default
    pixelart_body_squares(str2arrays(str), colors, options)
  end

  if pixelart._debug then
    -- Nothing
  else
    io.write(")")
  end
end

local function pixelart_tikzpixelart(coord, str, options)
  -- Parse options
  local options = parse(options, pixelart._default)

  if options.tikz then
    tex_sprint(string.format([[\begin{scope}[%s] ]], options.tikz))
  end
  tex_print(string.format(
    [[\begin{scope}[shift={%s}] ]],
    coord
  ))

  pixelart_body(str, options)

  tex_print([[\end{scope} ]])
  if options.tikz then
    tex_print([[\end{scope} ]])
  end
end

local function pixelart_pixelart(str, options)
  -- Parse options
  local options = parse(options, pixelart._default)
  -- Tikz environment
  tex_print([[\begin{tikzpicture}]])
  if options.tikz then
    tex_sprint(string.format("[%s]", options.tikz))
  end

  pixelart_body(str, options)

  tex_print([[\end{tikzpicture}]])
end

pixelart.pixelart = pixelart_pixelart
pixelart.tikzpixelart = pixelart_tikzpixelart
