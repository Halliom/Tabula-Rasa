function gen_world()

  local a = 1
  local c = 0

  while b < 65 do

    if c = 64 then
      a = a + 1
      c = 1
    end

    local b = math.random(0, 64)

    local i = 1

    while i <= a do
      world.add_block(a, i, c, 1)
      i = i + 1
    end

    c = c + 1

    i = list(a, b, c)
    print(i)

  end
end
