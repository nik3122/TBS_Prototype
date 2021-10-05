#pragma once


#include "TileOperator_Interface.generated.h"


class APassMap;


UINTERFACE(Blueprintable)
class PROTOTYPE_API UTileOperator_Interface :
	public UInterface
{
	GENERATED_BODY()
};


class PROTOTYPE_API ITileOperator_Interface
{
	GENERATED_BODY()

public:

    /** 
        To stop enumerating tiles OperateTile should return false.
    */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Unknown|ArenaFight")
	bool OperateTile(APassMap* pPassMap, const FIntPoint& tile);
};