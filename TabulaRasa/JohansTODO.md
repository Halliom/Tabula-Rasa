# Johans TODO-list (C++)

## Engine
 - Asset loading, reference counting (passing along and deleting when no one wants it anymore)
 - Input system (event based system that allows anyone to register a function callback from an input event)
 - World saving (create a file format for storing the data of a chunk)
 - Collision detection between player and world
 - Live script editing with hot reload

## Renderer
 - Implement a forward renderer again (sigh)
 - Fix SSAO
 - Maybe lower the chunk size for more efficient calculations for the meshing since it is now multithreaded and would enable easier updates for other things as well (maybe split several chunks into a super-chunk called a sector?)
 - For every chunk, generate a "plane" that encompasses all the blocks in the chunk and then use that plane for visibility testing for chunks

## Server
 - Implement server-client based architecture where the client is only responsible for rendering what the server gives it and the server is then responsible for most of the game code
