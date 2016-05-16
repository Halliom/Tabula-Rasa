function print_two(a, b)

  print(a.." "..b)

end

function gen_world()

  local r = 8
  local i = 1
  local j = 1
  local y = 1
  local x = 1

  while i <= 2*r do

    while j <= 2*r do
      print_two(j, i)

      if r*r == (i - y)*(i - y) + (j - x)*(j - x) then
        world.add_block(j, 1, i, 1)
        print_two(j, i)
      end

      j = j + 1
    end

    i = i + 1
  end
end
