#pragma once

enum TokenType
{
    TOKEN_START_PROGRAM
    , TOKEN_END_PROGRAM
    , TOKEN_DATA
    , TOKEN_COMMAND
    , TOKEN_STATEMENT
    , TOKEN_START_LIST  // 5
    , TOKEN_END_LIST
    , TOKEN_START_MAP
    , TOKEN_END_MAP
    , TOKEN_INTEGER
    , TOKEN_STRING    // 10
    , TOKEN_COMMENT
    , TOKEN_IF
    , TOKEN_THEN
    , TOKEN_ELSE
    , TOKEN_ENDIF   // 15
    , TOKEN_FOR
    , TOKEN_ENDFOR
    , TOKEN_EOL
};

struct Token
{
    std::string value;
    TokenType token;
};

enum ObjectType
{
    OBJECT_STRING
    ,OBJECT_INTEGER
    ,OBJECT_LIST
    ,OBJECT_MAP
    ,OBJECT_COMMAND
    ,OBJECT_PROGRAM
    ,OBJECT_TOKEN 
    ,OBJECT_IF
    ,OBJECT_FOR
};

