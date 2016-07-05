-- radien
local r = 4

-- mitt punkten av cirklen
local y0 = 0
local x0 = 0
local z0 = 0

-- antalet pungter
local i = 11
local j = 0

-- pungter inom randen
local list = {}

-- genererar random pungter
function p_generation()
  local k = {}
  local var = false
  while var == false do
    local y = math.random(0, r)
    local x = math.random(0, r)
    local z = math.random(0, r)
    if r^2 >= (y - y0)^2 + (x - x0)^2 + (z - z0)^2 then
      k = {y, x, z}
      var = true
      return k
    end
  end
end

while j <= i do
  list = list + p_generation()
  j =+ 1
end

-- soterar list efter punkternas avstånd från centrum
j = 0
while j <= i do
  local y = list[j[0]]
  local x = list[j[1]]
  local z = list[j[2]]
  local distance = math.sqrt((y - y0)^2 + (x - x0)^2 + (z - z0)^2)
  list[j[3]] = distance
  j =+ 1
end
