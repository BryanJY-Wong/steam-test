#include <steam/steam_api.h>
#include <chrono>
#include <cstdio>
#include <exception>
#include <iostream>
#include <fstream>

void create_app_file(int app_id)
{
    std::ofstream outfile;
    outfile.open("steam_appid.txt", std::ofstream::out | std::ofstream::trunc);
    outfile << app_id;
    outfile.flush();
    outfile.close();
    printf("AppID: %d\n", app_id);
}

struct Listeners
{
    STEAM_CALLBACK(Listeners, OnFriendStatusChanged, PersonaStateChange_t, m_FriendStatusChanged);
    Listeners() : m_FriendStatusChanged(this, &Listeners::OnFriendStatusChanged) {}
};
template<EPersonaChange N>
bool HasFlag(int f) { return f & N; }
void Listeners::OnFriendStatusChanged(PersonaStateChange_t *pParam)
{
    auto const name = SteamFriends()->GetFriendPersonaName(pParam->m_ulSteamID);
    printf("[%s] Friend: %s; Status %x\n", __FUNCTION__ , name, pParam->m_nChangeFlags);
    auto const is_self = SteamUser()->GetSteamID() == pParam->m_ulSteamID;
    ;
    if (!is_self && HasFlag<k_EPersonaChangeRelationshipChanged>(pParam->m_nChangeFlags))
    {
        printf("[%s] %s Friend %s\n", __FUNCTION__ ,
               SteamFriends()->HasFriend(pParam->m_ulSteamID, k_EFriendFlagImmediate) ? "Gained" : "Lost",
               name);
    }
}

bool IsOnline(CSteamID const& f)
{
    auto const state = SteamFriends()->GetFriendPersonaState(f);
    FriendGameInfo_t game_info;
    return state == k_EPersonaStateOnline &&
        SteamFriends()->GetFriendGamePlayed(f, &game_info) &&
        game_info.m_gameID.AppID() == SteamUtils()->GetAppID();
}

void PrintFriends()
{
    for (auto i = 0; i < SteamFriends()->GetFriendCount(k_EFriendFlagImmediate); ++i)
    {
        auto const name = SteamFriends()->GetFriendPersonaName(SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate));
        printf("[%s] Friend: %s\n", __FUNCTION__ , name);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) return -1;

    create_app_file(atoi(argv[1]));
    SteamAPI_Init();
    Listeners l;
    PrintFriends();
    auto const begin = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - begin < std::chrono::minutes(5))
    {
        SteamAPI_RunCallbacks();
    }

    return 0;
}