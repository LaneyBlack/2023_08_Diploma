#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Runtime/JsonUtilities/Public/JsonUtilities.h"

#include "EpicLeaderboardInterface.generated.h"

#define SERVER_URL "https://epicleaderboard.com"

class EPICLEADERBOARD_API EpicLeaderboardInterface
{
public:
	EpicLeaderboardInterface();
	~EpicLeaderboardInterface();
	
	static FString ConstructParamsURL(const TMap<FString, FString>& Params)
	{
		FString URL = "";
		
		if (Params.Num() > 0)
		{
			for (const auto& Param : Params)
			{
				FString EncodedKey = FGenericPlatformHttp::UrlEncode(Param.Key);
				FString EncodedValue = FGenericPlatformHttp::UrlEncode(Param.Value);
				URL.Append(FString::Printf(TEXT("%s=%s&"), *EncodedKey, *EncodedValue));
			}
			
			// Remove the trailing '&' character
			URL.RemoveFromEnd("&");
		}

		return URL;
	}
	
	static FString SerializeMap(const TMap<FString, FString> &metadata)
	{
		FString jsonStr;
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&jsonStr);
		JsonWriter->WriteObjectStart();

		for (const auto& Entry : metadata)
		{
			JsonWriter->WriteValue(Entry.Key, Entry.Value);
		}

		JsonWriter->WriteObjectEnd();
		JsonWriter->Close();

		return jsonStr;
	}

	static void DeserializeMap(FString json, TMap<FString, FString> &map)
	{
		map.Empty();

		TSharedPtr<FJsonValue> JsonParsed;
		TSharedRef< TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(json);
		if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
		{
			for (const auto& JsonEntry : JsonParsed->AsObject()->Values)
			{
				map.Add(JsonEntry.Key, JsonEntry.Value->AsString());
			}
		}
	}
};

USTRUCT(BlueprintType)
struct EPICLEADERBOARD_API FEpicLeaderboardGame
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GameID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GameKey;
};

USTRUCT(BlueprintType)
struct EPICLEADERBOARD_API FEpicLeaderboard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PrimaryID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SecondaryID;
};

USTRUCT(BlueprintType)
struct EPICLEADERBOARD_API FEpicLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Username;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Country;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Metadata;

	UPROPERTY()
	FString Meta;
};

USTRUCT(BlueprintType)
struct EPICLEADERBOARD_API FEpicLeaderboardGetEntriesResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEpicLeaderboardEntry> Entries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FEpicLeaderboardEntry PlayerEntry;
};

/*
enum Timeframe
{
	AllTime = 0,
	Year = 1,
	Month = 2,
	Week = 3,
	Day = 4
}
 */
UENUM(BlueprintType)
enum Timeframe : uint8
{
	AllTime = 0 UMETA(DisplayName = "All Time"),
	Year = 1 UMETA(DisplayName = "Year"),
	Month = 2 UMETA(DisplayName = "Month"),
	Week = 3 UMETA(DisplayName = "Week"),
	Day = 4 UMETA(DisplayName = "Day")
};

UENUM(BlueprintType)
enum IsUsernameAvailableResponse : uint8
{
	Available = 0 UMETA(DisplayName = "Available"),
	Invalid = 1 UMETA(DisplayName = "Invalid"),
	Profanity = 2 UMETA(DisplayName = "Profanity"),
	Taken = 3 UMETA(DisplayName = "Taken")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEpicLeaderboardGetEntriesEvent, FEpicLeaderboardGetEntriesResponse,
                                            Entries);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEpicLeaderboardGetEntriesErrorEvent);

UCLASS()
class EPICLEADERBOARD_API UGetLeaderboardEntries : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	FEpicLeaderboardGame Game;
	FEpicLeaderboard Leaderboard;
	Timeframe LeaderboardTimeFrame;
	FString Username;
	bool AroundPlayer;
	bool Local;

public:
	UPROPERTY(BlueprintAssignable)
	FEpicLeaderboardGetEntriesEvent Success;

	UPROPERTY(BlueprintAssignable)
	FEpicLeaderboardGetEntriesErrorEvent Error;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "EpicLeaderboard")
	static UGetLeaderboardEntries* GetLeaderboardEntries(
		FEpicLeaderboardGame Game,
		FEpicLeaderboard Leaderboard,
		FString Username,
		Timeframe Timeframe = Timeframe::AllTime,
		bool AroundPlayer = true,
		bool Local = false)
	{
		UGetLeaderboardEntries* BlueprintNode = NewObject<UGetLeaderboardEntries>();

		BlueprintNode->Game = Game;
		BlueprintNode->Leaderboard = Leaderboard;
		BlueprintNode->Username = Username;
		BlueprintNode->LeaderboardTimeFrame = Timeframe;
		BlueprintNode->AroundPlayer = AroundPlayer;
		BlueprintNode->Local = Local;
		
		return BlueprintNode;
	}

protected:
	virtual void Activate() override
	{
		//keep object alive after call ends
		//remove from root after the asnyc calls 
		this->AddToRoot();

		// Base URL of the API endpoint
		FString BaseURL = FString(TEXT(SERVER_URL)) + FString(TEXT("/api/getScores"));

		// List of query parameters
		TMap<FString, FString> QueryParams;
		QueryParams.Add(TEXT("gameID"), Game.GameID);
		QueryParams.Add(TEXT("primaryID"), Leaderboard.PrimaryID);
		QueryParams.Add(TEXT("secondaryID"), Leaderboard.SecondaryID);
		QueryParams.Add(TEXT("username"), Username);
		QueryParams.Add(TEXT("timeframe"), FString::FromInt((int32)LeaderboardTimeFrame));
		QueryParams.Add(TEXT("around"), AroundPlayer ? TEXT("1") : TEXT("0"));
		QueryParams.Add(TEXT("local"), Local ? TEXT("1") : TEXT("0"));

		// Construct the full URL with query parameters
		FString FullURL = BaseURL + "?" + EpicLeaderboardInterface::ConstructParamsURL(QueryParams);
		
		//setup the request
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &UGetLeaderboardEntries::OnHighscoreResponseReceived);

		Request->SetURL(FullURL);
		Request->SetVerb("GET");
		Request->SetHeader(
			TEXT("User-Agent"),
			"X-EpicLeaderboard UE5");
		Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
		Request->SetHeader("Accept", TEXT("application/json"));
		Request->ProcessRequest();
	}
	
	void OnHighscoreResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	                                                         bool bWasSuccessful)
	{
		this->RemoveFromRoot();
		
		if (!bWasSuccessful)
		{
			Error.Broadcast();
			return;
		}

		FString data = Response->GetContentAsString();

		//FString time = Response->GetHeader("X-Response-Time");
		//UE_LOG(LogTemp, Warning, TEXT("GetLeaderboardEntries Response Time: %s"), *time);

		FEpicLeaderboardGetEntriesResponse ResponseData;
		
		//parse json response
		TSharedPtr<FJsonValue> JsonParsed;
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(data);
		if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
		{
			//parse top score list 
			TSharedPtr<FJsonValue>* topScores = JsonParsed->AsObject()->Values.Find(TEXT("scores"));

			if (topScores != nullptr && topScores->IsValid())
			{
				//clear entries
				FJsonObjectConverter::JsonArrayToUStruct<FEpicLeaderboardEntry>(
					topScores->Get()->AsArray(), &ResponseData.Entries, 0, 0);

				//deserialize metadata
				for (auto& Entry : ResponseData.Entries)
				{
					EpicLeaderboardInterface::DeserializeMap(Entry.Meta, Entry.Metadata);
				}
			}

			//clear struct
			FEpicLeaderboardEntry PlayerEntry = FEpicLeaderboardEntry();

			//parse player score
			TSharedPtr<FJsonValue>* playerScore = JsonParsed->AsObject()->Values.Find(TEXT("playerscore"));

			if (playerScore != nullptr && playerScore->IsValid())
			{
				TSharedRef<FJsonObject> JsonObject = playerScore->Get()->AsObject().ToSharedRef();
				
				FJsonObjectConverter::JsonObjectToUStruct<FEpicLeaderboardEntry>(JsonObject, &PlayerEntry, 0, 0);
				
				//deserialize metadata
				EpicLeaderboardInterface::DeserializeMap(PlayerEntry.Meta, PlayerEntry.Metadata);
			}

			ResponseData.PlayerEntry = PlayerEntry;
		}

		Success.Broadcast(ResponseData);
	}
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEpicLeaderboardSubmitEntryEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEpicLeaderboardSubmitEntryErrorEvent);

UCLASS()
class EPICLEADERBOARD_API USubmitLeaderboardEntry : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	FEpicLeaderboardGame Game;
	FEpicLeaderboard Leaderboard;
	FString Username;
	double Score;
	TMap<FString, FString> Metadata;

public:
	UPROPERTY(BlueprintAssignable)
	FEpicLeaderboardSubmitEntryEvent Success;

	UPROPERTY(BlueprintAssignable)
	FEpicLeaderboardSubmitEntryErrorEvent Error;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "EpicLeaderboard")
	static USubmitLeaderboardEntry* SubmitLeaderboardEntryWithMetadata(
		FEpicLeaderboardGame Game,
		FEpicLeaderboard Leaderboard,
		FString Username,
		double Score,
		TMap<FString, FString> Metadata)
	{
		USubmitLeaderboardEntry* BlueprintNode = NewObject<USubmitLeaderboardEntry>();

		BlueprintNode->Game = Game;
		BlueprintNode->Leaderboard = Leaderboard;
		BlueprintNode->Username = Username;
		BlueprintNode->Score = Score;
		BlueprintNode->Metadata = Metadata;
		
		return BlueprintNode;
	}

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "EpicLeaderboard")
	static USubmitLeaderboardEntry* SubmitLeaderboardEntry(
		FEpicLeaderboardGame Game,
		FEpicLeaderboard Leaderboard,
		FString Username,
		double Score)
	{
		USubmitLeaderboardEntry* BlueprintNode = NewObject<USubmitLeaderboardEntry>();

		BlueprintNode->Game = Game;
		BlueprintNode->Leaderboard = Leaderboard;
		BlueprintNode->Username = Username;
		BlueprintNode->Score = Score;
		BlueprintNode->Metadata = TMap<FString, FString>();
		
		return BlueprintNode;
	}

protected:
	virtual void Activate() override
	{
		//keep object alive after call ends
		//remove from root after the asnyc calls 
		this->AddToRoot();

		// Base URL of the API endpoint
		FString BaseURL = FString(TEXT(SERVER_URL)) + FString(TEXT("/api/submitScore"));
		
		//setup the request
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &USubmitLeaderboardEntry::OnScoreSubmitResponseReceived);

		Request->SetURL(BaseURL);
		Request->SetVerb("POST");
		Request->SetHeader(
			TEXT("User-Agent"),
			"X-EpicLeaderboard UE5");
		Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
		Request->SetHeader("Accept", TEXT("application/json"));

		FString metaJson = EpicLeaderboardInterface::SerializeMap(Metadata);

		TMap<FString, FString> QueryParams;
		QueryParams.Add(TEXT("gameID"), Game.GameID);
		QueryParams.Add(TEXT("gameKey"), Game.GameKey);
		QueryParams.Add(TEXT("primaryID"), Leaderboard.PrimaryID);
		QueryParams.Add(TEXT("secondaryID"), Leaderboard.SecondaryID);
		QueryParams.Add(TEXT("username"), Username);
		QueryParams.Add(TEXT("score"), FString::Printf(TEXT("%.17g"), Score));
		QueryParams.Add(TEXT("meta"), metaJson);
		
		FString Content = EpicLeaderboardInterface::ConstructParamsURL(QueryParams);
		
		Request->SetContentAsString(Content);
		
		Request->ProcessRequest();
	}
	
	void OnScoreSubmitResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	                                                         bool bWasSuccessful)
	{
		this->RemoveFromRoot();
		
		if (!bWasSuccessful)
		{
			Error.Broadcast();
			return;
		}

		//FString time = Response->GetHeader("X-Response-Time");
		//UE_LOG(LogTemp, Warning, TEXT("SubmitLeaderboardEntry Response Time: %s"), *time);
		
		if(Response->GetResponseCode() == 200)
		{
			Success.Broadcast();
		}

		Error.Broadcast();
	}
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEpicLeaderboardIsUsernameAvailableEvent, IsUsernameAvailableResponse, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEpicLeaderboardIsUsernameAvailableErrorEvent);

UCLASS()
class EPICLEADERBOARD_API UIsUsernameAvailable : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	FEpicLeaderboardGame Game;
	FString Username;
	
public:
	UPROPERTY(BlueprintAssignable)
	FEpicLeaderboardIsUsernameAvailableEvent Success;

	UPROPERTY(BlueprintAssignable)
	FEpicLeaderboardIsUsernameAvailableErrorEvent Error;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "EpicLeaderboard")
	static UIsUsernameAvailable* IsUsernameAvailable(
		FEpicLeaderboardGame Game,
		FString Username)
	{
		UIsUsernameAvailable* BlueprintNode = NewObject<UIsUsernameAvailable>();

		BlueprintNode->Game = Game;
		BlueprintNode->Username = Username;
		
		return BlueprintNode;
	}

protected:
	virtual void Activate() override
	{
		//keep object alive after call ends
		//remove from root after the asnyc calls 
		this->AddToRoot();
		
		// Base URL of the API endpoint
		FString BaseURL = FString(TEXT(SERVER_URL)) + FString(TEXT("/api/isUsernameAvailable_v2"));

		// List of query parameters
		TMap<FString, FString> QueryParams;
		QueryParams.Add(TEXT("gameID"), Game.GameID);
		QueryParams.Add(TEXT("username"), Username);

		// Construct the full URL with query parameters
		FString FullURL = BaseURL + "?" + EpicLeaderboardInterface::ConstructParamsURL(QueryParams);
		
		//setup the request
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &UIsUsernameAvailable::OnResultReceived);

		Request->SetURL(FullURL);
		Request->SetVerb("GET");
		Request->SetHeader(
			TEXT("User-Agent"),
			"X-EpicLeaderboard UE5");
		Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
		Request->SetHeader("Accept", TEXT("application/json"));
		Request->ProcessRequest();
	}
	
	void OnResultReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	                                                         bool bWasSuccessful)
	{
		this->RemoveFromRoot();
		
		if (!bWasSuccessful)
		{
			Error.Broadcast();
			return;
		}

		//FString time = Response->GetHeader("X-Response-Time");
		//UE_LOG(LogTemp, Warning, TEXT("UIsUsernameAvailable Response Time: %s"), *time);
		
		FString data = Response->GetContentAsString();

		//convert string to enum
		IsUsernameAvailableResponse Result = IsUsernameAvailableResponse::Invalid;
		
		if(data == "0")
		{
			Result = IsUsernameAvailableResponse::Available;
		}
		else if(data == "2")
		{
			Result = IsUsernameAvailableResponse::Profanity;
		}
		else if(data == "3")
		{
			Result = IsUsernameAvailableResponse::Taken;
		}
		
		Success.Broadcast(Result);
	}
};