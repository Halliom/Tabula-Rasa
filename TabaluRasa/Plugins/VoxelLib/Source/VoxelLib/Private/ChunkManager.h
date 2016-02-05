#include "VoxelLibPluginPrivatePCH.h"

#include "ChunkManager.generated.h"

struct ChunkPos
{
	ChunkPos(int32 X, int32 Y, int32 Z) :
		PosX(X),
		PosY(Y),
		PosZ(Z)
	{ }

	ChunkPos(FWorldPosition Position) :
		PosX(Position.PositionX),
		PosY(Position.PositionY),
		PosZ(Position.PositionZ)
	{ }

	int32 PosX;
	int32 PosY;
	int32 PosZ;

	bool operator==(const ChunkPos& OtherPos) const
	{
		return (PosX == OtherPos.PosX) && (PosY == OtherPos.PosY) && (PosZ == OtherPos.PosZ);
	}

	bool operator==(ChunkPos& OtherPos) const
	{
		return (PosX == OtherPos.PosX) && (PosY == OtherPos.PosY) && (PosZ == OtherPos.PosZ);
	}

	friend FORCEINLINE uint32 GetTypeHash(const ChunkPos& Position)
	{
		return FCrc::MemCrc_DEPRECATED(&Position.PosX, sizeof(int32) * 3);
	}
};

UCLASS()
class AChunkManager : public AActor
{
	GENERATED_BODY()
public:

	AChunkManager(const FObjectInitializer& ObjectIntiailizer);

	static AChunkManager* GetStaticChunkManager();

	//UPROPERTY(Category = "World|Chunk", BlueprintCallable)
	AChunk* GetChunkFromPosition(FWorldPosition Position);

	void AddChunk(FWorldPosition ChunkPosition, AChunk* Chunk);

private:

	static AChunkManager* ChunkManager;

	TMap<ChunkPos, AChunk*> LoadedChunks;

};