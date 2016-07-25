-- radien
local r = 4

-- mitt punkten av cirklen
local xO = 0
local yO = 0
local zO = 0

-- antalet pungter
local i = 11
local j = 1

-- pungter inom randen
local list = {}

-- antal pungter som körs åt gången
local h = 3

-- sublits function
function coord(x, y, z)
  local result = {}
  result.x = x
  result.y = y
  result.z = z
  result.d = 0
  result.xn = 0
  result.yn = 0
  result.zn = 0
  result.should_be_removed = false
   alse
  return result
end

-- genererar random pungter
function p_generation()
  local k = {}
  local var = false
  while var == false do
    local x = math.random(-r, r)
    local y = math.random(-r, r)
    local z = math.random(-r, r)
    if r^2 >= (x - xO)^2 + (y - yO)^2 + (z - zO)^2 then
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
  local distance = math.sqrt((x - xO)^2 + (y - yO)^2 + (z - zO)^2)
  list[j].d = distance
  j = j + 1
end

function comapare(a, b)
  return a.d < b.d
end

table.sort(list, comapare)

-- breräknar en normaliserad vektor för x, y, z
j = 1
while j <= h do
  list[j].xn = (list[j].x - xO) / list[j].d
  list[j].yn = (list[j].y - yO) / list[j].d
  list[j].zn = (list[j].z - zO) / list[j].d

  if not list[j].xn == nil then
    list[j].xn = math.round(list[j].xn)
  end
  if not list[j].yn == nil then
    list[j].yn = math.round(list[j].yn)
  end
  if not list[j].zn == nil then
    list[j].zn = math.round(list[j].zn)
  end

  j = j + 1
end

for k, g in pairs(list) do
  print("-------")
  for h, l in pairs(g) do
    print(h.." : "..l)
  end
end

print("----------------------")

for a = (j - 3), j do
  for b = j, i do
    if 2 < (list[b].x - list[a].xn)^2 + (list[b].y - list[a].yn)^2 + (list[b].z - list[a].zn)^2 then
      list[j].should_be_removed = true
      print("GGGGGGGGGGGGG")
    end
  end
end

for a = 1, i do
  if list[a].should_be_removed == true then
    list[a].x = nil
    list[a].y = nil
    list[a].z = nil
    list[a].d = nil
    list[a].xn = nil
    list[a].yn = nil
    list[a].zn = nil
    result.should_be_removed = nil
  end
end

table.sort(list, comapare)

for k, g in pairs(list) do
  print("-------")
  for h, l in pairs(g) do
    print(h.." : "..l)
  end
end

print(#list)
