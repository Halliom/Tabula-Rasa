-- coordlist pungter inom randen
-- r radien
-- nub_coord antaet punkter som ska genereras
local r = 0
local nub_coord = 0
local y_high = 1
local coordlist = {}

function round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end

function angfunk(y)
  return y^2
end

function radifunk(y)
  return y^2
end

function GeneratTrees(xO, yO, zO, number_of_iterations, iteration_divider)
  -- xO, yO, zO utgåns punt för trädet
  -- angfunk beskriver hur den tillåtna vingklen ska öka
  -- radifunk brkriver ökningen av radien
  -- number of iterations antal iterationer

  local radius = function(y)
    local radi = round(radifunk(y))
    return radi
  end

  -- antalet pungter som genereras per lager
  local number_of_coords = function(y)
    local number = math.ceil((((radius(y)) ^ 2) * math.pi) / math.sqrt(r))
    return number
  end

  local coord = function(x, y, z)
    local result = {}
    result.x = x
    result.y = y
    result.z = z
    result.d = 0
    result.xn = 0
    result.yn = 0
    result.zn = 0
    result.should_be_removed = false
    result.suitability = 0
    return result
  end

  local coord_generation = function()
    local k = {}
    local var = false
    while var == false do
      local x = math.random(-r, r) + xO
      local y = math.random(-r, r) + yO
      local z = math.random(-r, r) + zO
      if r^2 >= (x)^2 + (y)^2 + (z)^2 and math.acos(y / (math.sqrt(x^2 + y^2 + z^2))) > angfunk(y) then
        k = coord(x, y, z)
        var = true
        return k
      end
    end
  end

  local block_placement = function()
    local number = math.random(1, #coordlist)

    coordlist[number].d = math.sqrt((coordlist[number].x)^2 + (coordlist[number].y)^2 + (coordlist[number].z)^2)

    coordlist[number].xn = coordlist[number].x / coordlist[number].d
    coordlist[number].yn = coordlist[number].y / coordlist[number].d
    coordlist[number].zn = coordlist[number].z / coordlist[number].d

    xO = coordlist[number].xn
    yO = coordlist[number].yn
    zO = coordlist[number].zn

    world.add_block(xO, yO, zO, 5)
    y_high = y_high + yO
  end

  local geraration_funktion = function(iterations)
    for i = 1, iterations do
      coordlist = {}
      r = radius(y_high)
      nub_coord = number_of_coords(y_high)
      for i = 1, nub_coord do
        table.insert(coordlist, coord_generation)
      end
      block_placement()
      if 60 < math.random(1, 100) then
        geraration_funktion(iterations / iteration_divider)
      end
    end
  end

  geraration_funktion(number_of_iterations)

end

GeneratTrees(3,3,3,50,5)
