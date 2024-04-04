#ifndef ENGINE_H
#define ENGINE_H
#define USERNAME_LENGTH 8

#define GAME_RED_TEAM 0
#define GAME_BLUE_TEAM 1
#define GAME_DEFAULT_LIFE_POINTS 25

#define OWASP_IDENTIFIER_LENGTH 8
#define OWASP_NAME_MAX_LENGTH 50

typedef enum {
    RED,
    BLUE
} TeamColor;
typedef struct {
    char username[USERNAME_LENGTH]; /*!< TEAM Board */
    int life_points; /*!< Life points */
    TeamColor team;   /*!< Team */
} PlayerInformation;

typedef struct {
    char cwe[3];
    char name[OWASP_NAME_MAX_LENGTH];
} OWASPVulns;
typedef struct {
    char identifier[OWASP_IDENTIFIER_LENGTH];
    char name[OWASP_NAME_MAX_LENGTH];
}OWASPBaseAction;
typedef struct {
    OWASPBaseAction *attack;
    OWASPBaseAction *response;
}OWASPAction;
typedef struct {
    OWASPVulns *vuln;
    OWASPAction *action1;
    OWASPAction *action2;
} OWASPProfile;


typedef enum {
    PAIRING=0,
    PROFILE_SELECTOR,
    WAITING_ATTACK,
    SEND_ATTACK,
    WAITING_RESPONSE,
    FINISHED,
} GameState;


typedef struct {
    GameState game_state;
    PlayerInformation player;
    OWASPProfile *profile;
} GameInformation;


extern OWASPProfile OWASPA01;
extern OWASPProfile OWASPA02;
extern OWASPProfile OWASPA03;
extern OWASPProfile OWASPA04;
extern OWASPProfile OWASPA05;
extern OWASPProfile OWASPA06;
extern OWASPProfile OWASPA07;
extern OWASPProfile OWASPA08;
extern OWASPProfile OWASPA09;
extern OWASPProfile OWASPA10;

extern OWASPProfile* owasp_profiles[];
OWASPProfile* getOWASPProfile(int index);

extern void get_owasp_profile_cwe_uint8(uint8_t* dest_buffer, OWASPProfile *profile);
extern void create_owasp_commmand(uint8_t* dest_buffer, OWASPProfile *profile);

#endif // ENGINE_H