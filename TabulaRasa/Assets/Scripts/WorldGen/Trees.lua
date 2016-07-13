-- radien
local r = 4

-- mitt punkten av cirklen
local x0 = 0
local y0 = 0
local z0 = 0

-- antalet pungter
local i = 11
local j = 1

-- pungter inom randen
local list = {}

-- sublits function
function coord(x, y, z)
  local result = {}
  result.x = x
  result.y = y
  result.z = z
  result.d = 0
  result.v = 0
  return result
end

-- genererar random pungter
function p_generation()
  local k = {}
  local var = false
  while var == false do
    local x = math.random(0, r)
    local y = math.random(0, r)
    local z = math.random(0, r)
    if r^2 >= (x - x0)^2 + (y - y0)^2 + (z - z0)^2 then
      k = coord(x, y, z)
      var = true
      return k
    end
  end
end

while j <= i do
  table.insert(list, p_generation())
  j = j + 1
end

-- soterar list efter punkternas avstånd från centrum
j = 1
while j <= i do
  local x = list[j].x
  local y = list[j].y
  local z = list[j].z
  local distance = math.sqrt((x - x0)^2 + (y - y0)^2 + (z - z0)^2)
  list[j].d = distance
  j = j + 1
end

function comapare(a, b)
  return a.d < b.d
end

for k, g in pairs(list) do
  for h, l in pairs(g) do
    print(h.." : "..l)
  end
end

table.sort(list, comapare)

print("----------------")

for k, g in pairs(list) do
  for h, l in pairs(g) do
    print(h.." : "..l)
  end
end
