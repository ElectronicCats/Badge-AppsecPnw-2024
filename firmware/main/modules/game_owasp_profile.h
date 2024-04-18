#include "modules/game_engine_module.h"
#ifndef GAME_OWASP_PROFILE_H
  #define GAME_OWASP_PROFILE_H
  #define GAME_OWASP_PROFILES_COUNT 10
game_owasp_vulns_t owasp_game_vulns[] = {
    {"A01", "Broken Access Control"},
    {"A02", "Cryptographic Failures"},
    {"A03", "Injection"},
    {"A04", "Insecure Design"},
    {"A05", "Security Misconfiguration"},
    {"A06", "Vulnerable outdated Components"},
    {"A07", "Identification Authentication Failures"},
    {"A08", "Software and Data Integrity Failures"},
    {"A09", "Security Logging and Monitoring Failures"},
    {"A10", "Server Side Request Forgery"}};

game_owasp_base_actions_t owasp_game_profile_base_actions[] = {
    {"CWE-25", "Path Traversal"},
    {"CWE-25R", "Validate Input"},
    {"CWE-352", "Cross-Site Request Forgery"},
    {"CWE-352R", "Anti-CSRF"},
    {"CWE-261", "Weak Encoding Password"},
    {"CWE-261R", "128 bits Encryption"},
    {"CWE-319", "Cleartext Transmission"},
    {"CWE-319R", "SSL/Cryptographic"},
    {"CWE-20", "Improper Input Validation"},
    {"CWE-20R", "Client/Server Validation"},
    {"CWE-89", "SQL Injection"},
    {"CWE-89R", "Isolated account"},
    {"CWE-256", "Unprotected Storage of Credentials"},
    {"CWE-256R", "Hashes of password"},
    {"CWE-434", "Unrestricted Upload"},
    {"CWE-434R", "Generate Hashes of Files"},
    {"CWE-260", "Password in Configuration File"},
    {"CWE-260R", "Static Application Security Testing"},
    {"CWE-315", "Cleartext storage"},
    {"CWE-315R", "Encrypt Information"},
    {"CWE-937", "Using Commponents with Known Vulns"},
    {"CWE-937R", "Update Version"},
    {"CWE-319", "Use of Unmaintained Commponents"},
    {"CWE-319R", "Change Component"},
    {"CWE-259", "Use of Hardcoded Credentials"},
    {"CWE-259R", "Store password outside of code"},
    {"CWE-307", "Improper Handler Authentication Attempts"},
    {"CWE-307R", "Implemment Lockout Mechanism"},
    {"CWE-426", "Untrusted Search Path"},
    {"CWE-426R", "Use function that require explicit path"},
    {"CWE-494", "Download of Code Without Integrity Check"},
    {"CWE-494R", "Implement Integrity Check"},
    {"CWE-532", "Insertion of Sensitive Information"},
    {"CWE-532R", "Protect Log files from unauthorized access"},
    {"CWE-778", "Insufficient Logging"},
    {"CWE-778R", "Centralized logging mmechanism"},
    {"CWE-918", "Server-Side Request Forgery"},
    {"CWE-918R", "Restrict access to internal network"}};

game_owasp_actions_t owasp_game_actions_profile[] = {
    {&owasp_game_profile_base_actions[0],
     &owasp_game_profile_base_actions[1]},  // CWE-25, CWE-25R
    {&owasp_game_profile_base_actions[2],
     &owasp_game_profile_base_actions[3]},  // CWE-352, CWE-352R
    {&owasp_game_profile_base_actions[4],
     &owasp_game_profile_base_actions[5]},  // CWE-261, CWE-261R
    {&owasp_game_profile_base_actions[6],
     &owasp_game_profile_base_actions[7]},  // CWE-319, CWE-319R
    {&owasp_game_profile_base_actions[8],
     &owasp_game_profile_base_actions[9]},  // CWE-20, CWE-20R
    {&owasp_game_profile_base_actions[10],
     &owasp_game_profile_base_actions[11]},  // CWE-89, CWE-89R
    {&owasp_game_profile_base_actions[12],
     &owasp_game_profile_base_actions[13]},  // CWE-256, CWE-256R
    {&owasp_game_profile_base_actions[14],
     &owasp_game_profile_base_actions[15]},  // CWE-434, CWE-434R
    {&owasp_game_profile_base_actions[16],
     &owasp_game_profile_base_actions[17]},  // CWE-260, CWE-260R
    {&owasp_game_profile_base_actions[18],
     &owasp_game_profile_base_actions[19]},  // CWE-315, CWE-315R
    {&owasp_game_profile_base_actions[20],
     &owasp_game_profile_base_actions[21]},  // CWE-937, CWE-937R
    {&owasp_game_profile_base_actions[22],
     &owasp_game_profile_base_actions[23]},  // CWE-319, CWE-319R
    {&owasp_game_profile_base_actions[24],
     &owasp_game_profile_base_actions[25]},  // CWE-259, CWE-259R
    {&owasp_game_profile_base_actions[26],
     &owasp_game_profile_base_actions[27]},  // CWE-307, CWE-307R
    {&owasp_game_profile_base_actions[28],
     &owasp_game_profile_base_actions[29]},  // CWE-426, CWE-426R
    {&owasp_game_profile_base_actions[30],
     &owasp_game_profile_base_actions[31]},  // CWE-494, CWE-494R
    {&owasp_game_profile_base_actions[32],
     &owasp_game_profile_base_actions[33]},  // CWE-532, CWE-532R
    {&owasp_game_profile_base_actions[34],
     &owasp_game_profile_base_actions[35]},  // CWE-778, CWE-778R
    {&owasp_game_profile_base_actions[36],
     &owasp_game_profile_base_actions[37]},  // CWE-918, CWE-918R
};

game_owasp_profile_t owasp_game_profiles[] = {
    {&owasp_game_vulns[0], &owasp_game_actions_profile[0],
     &owasp_game_actions_profile[1]},  // A01
    {&owasp_game_vulns[1], &owasp_game_actions_profile[2],
     &owasp_game_actions_profile[3]},  // A02
    {&owasp_game_vulns[2], &owasp_game_actions_profile[4],
     &owasp_game_actions_profile[5]},  // A03
    {&owasp_game_vulns[3], &owasp_game_actions_profile[6],
     &owasp_game_actions_profile[7]},  // A04
    {&owasp_game_vulns[4], &owasp_game_actions_profile[8],
     &owasp_game_actions_profile[9]},  // A05
    {&owasp_game_vulns[5], &owasp_game_actions_profile[10],
     &owasp_game_actions_profile[11]},  // A06
    {&owasp_game_vulns[6], &owasp_game_actions_profile[12],
     &owasp_game_actions_profile[13]},  // A07
    {&owasp_game_vulns[7], &owasp_game_actions_profile[14],
     &owasp_game_actions_profile[15]},  // A08
    {&owasp_game_vulns[8], &owasp_game_actions_profile[16],
     &owasp_game_actions_profile[17]},  // A09
    {&owasp_game_vulns[9], &owasp_game_actions_profile[18],
     &owasp_game_actions_profile[19]}  // A10
};
#endif  // GAME_OWASP_PROFILE_H
