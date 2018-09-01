#pragma once

enum Token
{
    TOKEN_START_PROGRAM
    , TOKEN_END_PROGRAM
    , TOKEN_DATA
    , TOKEN_COMMAND
    , TOKEN_START_LIST
    , TOKEN_END_LIST
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
};

