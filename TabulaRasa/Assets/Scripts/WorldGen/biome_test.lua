BiomeTest = {
	min_heat = 0,
	max_heat = 50,
	min_height = -128,
	max_height = -1,
	generate = function(simplex_noise)
		print("Hello world from the Test Biome!")
		print(simplex_noise:noise(3.0, 0.0))
	end
}