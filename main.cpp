#include <steam/steam_api.h>
#include <chrono>
#include <cstdio>
#include <exception>
#include <iostream>
#include <fstream>
#include <eos_sdk.h>
#include <eos_logging.h>

// char const EOS_PRODUCT_ID[] = "ec72fbf877284381a51d0d19d5358df9";
// char const EOS_SANDBOX_ID[] = "2c42520d342a46d7a6e0cfa77b4715de";
// char const EOS_DEPLOYMENT_ID[] = "2a8b4f6deb8f46f885234e9688f89659";
// char const EOS_CLIENT_ID[] = "xyza7891WtgGtSuiGHqI3IhVHFxQZFHW";
// char const EOS_CLIENT_SECRET[] = "dwA9gxRXvxP0UjhEHLHqKxSyuHjOY0ZkJNtW/IlCOK0";
char const EOS_PRODUCT_ID[] = "f620e9391719434c98728ab59cbb31f8";
char const EOS_SANDBOX_ID[] = "07250d7e0f754f889319f43d4d0d57ed";
char const EOS_DEPLOYMENT_ID[] = "fc88d06df7534c6c8d178d1b54bf7011";
char const EOS_CLIENT_ID[] = "xyza7891iJ0TPJbX9F8TB6KwhMvf8vx6";
char const EOS_CLIENT_SECRET[] = "ZeBdw/25HefJoa/t8LLbwdoRtE0iRWZ6Ql0BPQEefUs";
char const PRODUCT_NAME[] = "DL";

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

struct EOS_Module
{
    CCallResult<EOS_Module, EncryptedAppTicketResponse_t> m_SteamResult;
    EOS_HPlatform platform;
    EOS_Module()
    {
        EOS_InitializeOptions init{};
        init.ApiVersion = EOS_INITIALIZE_API_LATEST;
        init.ProductVersion = "0.1";
        init.ProductName = PRODUCT_NAME;
        auto const result = EOS_Initialize(&init);
        puts(EOS_EResult_ToString(result));
        assert(result == EOS_EResult::EOS_Success);
        EOS_Logging_SetLogLevel(EOS_ELogCategory::EOS_LC_ALL_CATEGORIES, EOS_ELogLevel::EOS_LOG_VeryVerbose);
        EOS_Logging_SetCallback([](EOS_LogMessage const* m)
            {
                printf("[EOS %s] %s\n", m->Category, m->Message);
            });
        EOS_Platform_Options options{};
        options.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
        options.ClientCredentials.ClientId = EOS_CLIENT_ID;
        options.ClientCredentials.ClientSecret = EOS_CLIENT_SECRET;
        options.ProductId = EOS_PRODUCT_ID;
        options.SandboxId = EOS_SANDBOX_ID;
        options.DeploymentId = EOS_DEPLOYMENT_ID;
        platform = EOS_Platform_Create(&options);
        assert(platform);
        LoginSteam();
    }

    void LoginSteam()
    {
        m_SteamResult.Set(SteamUser()->RequestEncryptedAppTicket(nullptr, 0), this, &EOS_Module::OnAppTicket);
    }

    void OnAppTicket(EncryptedAppTicketResponse_t* response, bool failed)
    {
        assert(!failed);
        assert(response->m_eResult == k_EResultOK);
        static uint8_t buffer[1024];
        uint32_t size = sizeof(buffer);
        auto const result = SteamUser()->GetEncryptedAppTicket(buffer, size, &size);
        assert(result);
        Login(buffer, size);
    }

    void Login(uint8_t const* bytes, uint32_t size)
    {
        static char buffer[2049];
        uint32_t buf_size = sizeof(buffer);    
        assert(bytes);
        printf("size %d\n", size);
        auto const result = EOS_ByteArray_ToString(bytes, size, buffer, &buf_size);
        printf("Hex String %s\n", EOS_EResult_ToString(result));
        assert(result == EOS_EResult::EOS_Success);

        auto connect = EOS_Platform_GetConnectInterface(platform);
        EOS_Connect_LoginOptions options{};
        EOS_Connect_Credentials cred{};
        cred.Token = buffer;
        cred.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
        cred.Type = EOS_EExternalCredentialType::EOS_ECT_STEAM_APP_TICKET;
        options.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
        options.Credentials = &cred;
        EOS_Connect_Login(connect, &options, this, [](EOS_Connect_LoginCallbackInfo const* info)
        {
            printf("%s %s\n", __PRETTY_FUNCTION__, EOS_EResult_ToString(info->ResultCode));
            assert(info->ResultCode == EOS_EResult::EOS_Success);
        });
        puts("Logging In via EOS Connect");
    }

    void OnLogin(EOS_Connect_LoginCallbackInfo const* info)
    {
        char str[129];
        int32_t size = sizeof(str);
        EOS_ProductUserId_ToString(info->LocalUserId, str, &size);
        printf("[%s] %s\n", __PRETTY_FUNCTION__, str);
    }

    ~EOS_Module()
    {
        EOS_Shutdown();
    }
    
};

int main(int argc, char** argv)
{
    if (argc < 2) return -1;

    create_app_file(atoi(argv[1]));
    SteamAPI_Init();
    puts("Begin");
    Listeners l;
    EOS_Module e;
    PrintFriends();
    auto const begin = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - begin < std::chrono::minutes(5))
    {
        SteamAPI_RunCallbacks();
        EOS_Platform_Tick(e.platform);
    }

    return 0;
}