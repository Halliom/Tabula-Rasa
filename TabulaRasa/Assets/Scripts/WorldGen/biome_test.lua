BiomeTest = {
	min_heat = 0,
	max_heat = 50,
	min_height = -64,
	max_height = 1,
	features = {
		"BaseTerrain"
	}
}

BaseTerrain = {
	generate = function(simplex_noise, chunk, world_x, world_y, world_z)
		for i = 0, 31 do
			for j = 0, 31 do
				local Y = simplex_noise:noise((world_x + i) / 50.0, (world_z + j) / 50.0) * 10
				for k = 0, 31 do
					if (world_y + k) <= Y then
						chunk:set_block(i, k, j, 1)
					end
				end
			end
		end
	end
}