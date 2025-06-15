/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED
# define YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    CONST = 258,                   /* CONST  */
    BOOL = 259,                    /* BOOL  */
    INT = 260,                     /* INT  */
    UINT = 261,                    /* UINT  */
    FLOAT = 262,                   /* FLOAT  */
    BVEC2 = 263,                   /* BVEC2  */
    BVEC3 = 264,                   /* BVEC3  */
    BVEC4 = 265,                   /* BVEC4  */
    IVEC2 = 266,                   /* IVEC2  */
    IVEC3 = 267,                   /* IVEC3  */
    IVEC4 = 268,                   /* IVEC4  */
    UVEC2 = 269,                   /* UVEC2  */
    UVEC3 = 270,                   /* UVEC3  */
    UVEC4 = 271,                   /* UVEC4  */
    VEC2 = 272,                    /* VEC2  */
    VEC3 = 273,                    /* VEC3  */
    VEC4 = 274,                    /* VEC4  */
    MAT2 = 275,                    /* MAT2  */
    MAT3 = 276,                    /* MAT3  */
    MAT4 = 277,                    /* MAT4  */
    MAT2X2 = 278,                  /* MAT2X2  */
    MAT2X3 = 279,                  /* MAT2X3  */
    MAT2X4 = 280,                  /* MAT2X4  */
    MAT3X2 = 281,                  /* MAT3X2  */
    MAT3X3 = 282,                  /* MAT3X3  */
    MAT3X4 = 283,                  /* MAT3X4  */
    MAT4X2 = 284,                  /* MAT4X2  */
    MAT4X3 = 285,                  /* MAT4X3  */
    MAT4X4 = 286,                  /* MAT4X4  */
    SAMPLER2D = 287,               /* SAMPLER2D  */
    SAMPLER3D = 288,               /* SAMPLER3D  */
    SAMPLERCUBE = 289,             /* SAMPLERCUBE  */
    SAMPLER2DSHADOW = 290,         /* SAMPLER2DSHADOW  */
    SAMPLERCUBESHADOW = 291,       /* SAMPLERCUBESHADOW  */
    SAMPLER2DARRAY = 292,          /* SAMPLER2DARRAY  */
    SAMPLER2DARRAYSHADOW = 293,    /* SAMPLER2DARRAYSHADOW  */
    ISAMPLER2D = 294,              /* ISAMPLER2D  */
    ISAMPLER3D = 295,              /* ISAMPLER3D  */
    ISAMPLERCUBE = 296,            /* ISAMPLERCUBE  */
    ISAMPLER2DARRAY = 297,         /* ISAMPLER2DARRAY  */
    USAMPLER2D = 298,              /* USAMPLER2D  */
    USAMPLER3D = 299,              /* USAMPLER3D  */
    USAMPLERCUBE = 300,            /* USAMPLERCUBE  */
    USAMPLER2DARRAY = 301,         /* USAMPLER2DARRAY  */
    SAMPLER = 302,                 /* SAMPLER  */
    SAMPLERSHADOW = 303,           /* SAMPLERSHADOW  */
    TEXTURE2D = 304,               /* TEXTURE2D  */
    TEXTURE3D = 305,               /* TEXTURE3D  */
    TEXTURECUBE = 306,             /* TEXTURECUBE  */
    TEXTURE2DARRAY = 307,          /* TEXTURE2DARRAY  */
    ITEXTURE2D = 308,              /* ITEXTURE2D  */
    ITEXTURE3D = 309,              /* ITEXTURE3D  */
    ITEXTURECUBE = 310,            /* ITEXTURECUBE  */
    ITEXTURE2DARRAY = 311,         /* ITEXTURE2DARRAY  */
    UTEXTURE2D = 312,              /* UTEXTURE2D  */
    UTEXTURE3D = 313,              /* UTEXTURE3D  */
    UTEXTURECUBE = 314,            /* UTEXTURECUBE  */
    UTEXTURE2DARRAY = 315,         /* UTEXTURE2DARRAY  */
    ATTRIBUTE = 316,               /* ATTRIBUTE  */
    VARYING = 317,                 /* VARYING  */
    FLOATE5M2_T = 318,             /* FLOATE5M2_T  */
    FLOATE4M3_T = 319,             /* FLOATE4M3_T  */
    BFLOAT16_T = 320,              /* BFLOAT16_T  */
    FLOAT16_T = 321,               /* FLOAT16_T  */
    FLOAT32_T = 322,               /* FLOAT32_T  */
    DOUBLE = 323,                  /* DOUBLE  */
    FLOAT64_T = 324,               /* FLOAT64_T  */
    INT64_T = 325,                 /* INT64_T  */
    UINT64_T = 326,                /* UINT64_T  */
    INT32_T = 327,                 /* INT32_T  */
    UINT32_T = 328,                /* UINT32_T  */
    INT16_T = 329,                 /* INT16_T  */
    UINT16_T = 330,                /* UINT16_T  */
    INT8_T = 331,                  /* INT8_T  */
    UINT8_T = 332,                 /* UINT8_T  */
    I64VEC2 = 333,                 /* I64VEC2  */
    I64VEC3 = 334,                 /* I64VEC3  */
    I64VEC4 = 335,                 /* I64VEC4  */
    U64VEC2 = 336,                 /* U64VEC2  */
    U64VEC3 = 337,                 /* U64VEC3  */
    U64VEC4 = 338,                 /* U64VEC4  */
    I32VEC2 = 339,                 /* I32VEC2  */
    I32VEC3 = 340,                 /* I32VEC3  */
    I32VEC4 = 341,                 /* I32VEC4  */
    U32VEC2 = 342,                 /* U32VEC2  */
    U32VEC3 = 343,                 /* U32VEC3  */
    U32VEC4 = 344,                 /* U32VEC4  */
    I16VEC2 = 345,                 /* I16VEC2  */
    I16VEC3 = 346,                 /* I16VEC3  */
    I16VEC4 = 347,                 /* I16VEC4  */
    U16VEC2 = 348,                 /* U16VEC2  */
    U16VEC3 = 349,                 /* U16VEC3  */
    U16VEC4 = 350,                 /* U16VEC4  */
    I8VEC2 = 351,                  /* I8VEC2  */
    I8VEC3 = 352,                  /* I8VEC3  */
    I8VEC4 = 353,                  /* I8VEC4  */
    U8VEC2 = 354,                  /* U8VEC2  */
    U8VEC3 = 355,                  /* U8VEC3  */
    U8VEC4 = 356,                  /* U8VEC4  */
    DVEC2 = 357,                   /* DVEC2  */
    DVEC3 = 358,                   /* DVEC3  */
    DVEC4 = 359,                   /* DVEC4  */
    DMAT2 = 360,                   /* DMAT2  */
    DMAT3 = 361,                   /* DMAT3  */
    DMAT4 = 362,                   /* DMAT4  */
    BF16VEC2 = 363,                /* BF16VEC2  */
    BF16VEC3 = 364,                /* BF16VEC3  */
    BF16VEC4 = 365,                /* BF16VEC4  */
    FE5M2VEC2 = 366,               /* FE5M2VEC2  */
    FE5M2VEC3 = 367,               /* FE5M2VEC3  */
    FE5M2VEC4 = 368,               /* FE5M2VEC4  */
    FE4M3VEC2 = 369,               /* FE4M3VEC2  */
    FE4M3VEC3 = 370,               /* FE4M3VEC3  */
    FE4M3VEC4 = 371,               /* FE4M3VEC4  */
    F16VEC2 = 372,                 /* F16VEC2  */
    F16VEC3 = 373,                 /* F16VEC3  */
    F16VEC4 = 374,                 /* F16VEC4  */
    F16MAT2 = 375,                 /* F16MAT2  */
    F16MAT3 = 376,                 /* F16MAT3  */
    F16MAT4 = 377,                 /* F16MAT4  */
    F32VEC2 = 378,                 /* F32VEC2  */
    F32VEC3 = 379,                 /* F32VEC3  */
    F32VEC4 = 380,                 /* F32VEC4  */
    F32MAT2 = 381,                 /* F32MAT2  */
    F32MAT3 = 382,                 /* F32MAT3  */
    F32MAT4 = 383,                 /* F32MAT4  */
    F64VEC2 = 384,                 /* F64VEC2  */
    F64VEC3 = 385,                 /* F64VEC3  */
    F64VEC4 = 386,                 /* F64VEC4  */
    F64MAT2 = 387,                 /* F64MAT2  */
    F64MAT3 = 388,                 /* F64MAT3  */
    F64MAT4 = 389,                 /* F64MAT4  */
    DMAT2X2 = 390,                 /* DMAT2X2  */
    DMAT2X3 = 391,                 /* DMAT2X3  */
    DMAT2X4 = 392,                 /* DMAT2X4  */
    DMAT3X2 = 393,                 /* DMAT3X2  */
    DMAT3X3 = 394,                 /* DMAT3X3  */
    DMAT3X4 = 395,                 /* DMAT3X4  */
    DMAT4X2 = 396,                 /* DMAT4X2  */
    DMAT4X3 = 397,                 /* DMAT4X3  */
    DMAT4X4 = 398,                 /* DMAT4X4  */
    F16MAT2X2 = 399,               /* F16MAT2X2  */
    F16MAT2X3 = 400,               /* F16MAT2X3  */
    F16MAT2X4 = 401,               /* F16MAT2X4  */
    F16MAT3X2 = 402,               /* F16MAT3X2  */
    F16MAT3X3 = 403,               /* F16MAT3X3  */
    F16MAT3X4 = 404,               /* F16MAT3X4  */
    F16MAT4X2 = 405,               /* F16MAT4X2  */
    F16MAT4X3 = 406,               /* F16MAT4X3  */
    F16MAT4X4 = 407,               /* F16MAT4X4  */
    F32MAT2X2 = 408,               /* F32MAT2X2  */
    F32MAT2X3 = 409,               /* F32MAT2X3  */
    F32MAT2X4 = 410,               /* F32MAT2X4  */
    F32MAT3X2 = 411,               /* F32MAT3X2  */
    F32MAT3X3 = 412,               /* F32MAT3X3  */
    F32MAT3X4 = 413,               /* F32MAT3X4  */
    F32MAT4X2 = 414,               /* F32MAT4X2  */
    F32MAT4X3 = 415,               /* F32MAT4X3  */
    F32MAT4X4 = 416,               /* F32MAT4X4  */
    F64MAT2X2 = 417,               /* F64MAT2X2  */
    F64MAT2X3 = 418,               /* F64MAT2X3  */
    F64MAT2X4 = 419,               /* F64MAT2X4  */
    F64MAT3X2 = 420,               /* F64MAT3X2  */
    F64MAT3X3 = 421,               /* F64MAT3X3  */
    F64MAT3X4 = 422,               /* F64MAT3X4  */
    F64MAT4X2 = 423,               /* F64MAT4X2  */
    F64MAT4X3 = 424,               /* F64MAT4X3  */
    F64MAT4X4 = 425,               /* F64MAT4X4  */
    ATOMIC_UINT = 426,             /* ATOMIC_UINT  */
    ACCSTRUCTNV = 427,             /* ACCSTRUCTNV  */
    ACCSTRUCTEXT = 428,            /* ACCSTRUCTEXT  */
    RAYQUERYEXT = 429,             /* RAYQUERYEXT  */
    FCOOPMATNV = 430,              /* FCOOPMATNV  */
    ICOOPMATNV = 431,              /* ICOOPMATNV  */
    UCOOPMATNV = 432,              /* UCOOPMATNV  */
    COOPMAT = 433,                 /* COOPMAT  */
    COOPVECNV = 434,               /* COOPVECNV  */
    HITOBJECTNV = 435,             /* HITOBJECTNV  */
    HITOBJECTATTRNV = 436,         /* HITOBJECTATTRNV  */
    TENSORLAYOUTNV = 437,          /* TENSORLAYOUTNV  */
    TENSORVIEWNV = 438,            /* TENSORVIEWNV  */
    SAMPLERCUBEARRAY = 439,        /* SAMPLERCUBEARRAY  */
    SAMPLERCUBEARRAYSHADOW = 440,  /* SAMPLERCUBEARRAYSHADOW  */
    ISAMPLERCUBEARRAY = 441,       /* ISAMPLERCUBEARRAY  */
    USAMPLERCUBEARRAY = 442,       /* USAMPLERCUBEARRAY  */
    SAMPLER1D = 443,               /* SAMPLER1D  */
    SAMPLER1DARRAY = 444,          /* SAMPLER1DARRAY  */
    SAMPLER1DARRAYSHADOW = 445,    /* SAMPLER1DARRAYSHADOW  */
    ISAMPLER1D = 446,              /* ISAMPLER1D  */
    SAMPLER1DSHADOW = 447,         /* SAMPLER1DSHADOW  */
    SAMPLER2DRECT = 448,           /* SAMPLER2DRECT  */
    SAMPLER2DRECTSHADOW = 449,     /* SAMPLER2DRECTSHADOW  */
    ISAMPLER2DRECT = 450,          /* ISAMPLER2DRECT  */
    USAMPLER2DRECT = 451,          /* USAMPLER2DRECT  */
    SAMPLERBUFFER = 452,           /* SAMPLERBUFFER  */
    ISAMPLERBUFFER = 453,          /* ISAMPLERBUFFER  */
    USAMPLERBUFFER = 454,          /* USAMPLERBUFFER  */
    SAMPLER2DMS = 455,             /* SAMPLER2DMS  */
    ISAMPLER2DMS = 456,            /* ISAMPLER2DMS  */
    USAMPLER2DMS = 457,            /* USAMPLER2DMS  */
    SAMPLER2DMSARRAY = 458,        /* SAMPLER2DMSARRAY  */
    ISAMPLER2DMSARRAY = 459,       /* ISAMPLER2DMSARRAY  */
    USAMPLER2DMSARRAY = 460,       /* USAMPLER2DMSARRAY  */
    SAMPLEREXTERNALOES = 461,      /* SAMPLEREXTERNALOES  */
    SAMPLEREXTERNAL2DY2YEXT = 462, /* SAMPLEREXTERNAL2DY2YEXT  */
    ISAMPLER1DARRAY = 463,         /* ISAMPLER1DARRAY  */
    USAMPLER1D = 464,              /* USAMPLER1D  */
    USAMPLER1DARRAY = 465,         /* USAMPLER1DARRAY  */
    F16SAMPLER1D = 466,            /* F16SAMPLER1D  */
    F16SAMPLER2D = 467,            /* F16SAMPLER2D  */
    F16SAMPLER3D = 468,            /* F16SAMPLER3D  */
    F16SAMPLER2DRECT = 469,        /* F16SAMPLER2DRECT  */
    F16SAMPLERCUBE = 470,          /* F16SAMPLERCUBE  */
    F16SAMPLER1DARRAY = 471,       /* F16SAMPLER1DARRAY  */
    F16SAMPLER2DARRAY = 472,       /* F16SAMPLER2DARRAY  */
    F16SAMPLERCUBEARRAY = 473,     /* F16SAMPLERCUBEARRAY  */
    F16SAMPLERBUFFER = 474,        /* F16SAMPLERBUFFER  */
    F16SAMPLER2DMS = 475,          /* F16SAMPLER2DMS  */
    F16SAMPLER2DMSARRAY = 476,     /* F16SAMPLER2DMSARRAY  */
    F16SAMPLER1DSHADOW = 477,      /* F16SAMPLER1DSHADOW  */
    F16SAMPLER2DSHADOW = 478,      /* F16SAMPLER2DSHADOW  */
    F16SAMPLER1DARRAYSHADOW = 479, /* F16SAMPLER1DARRAYSHADOW  */
    F16SAMPLER2DARRAYSHADOW = 480, /* F16SAMPLER2DARRAYSHADOW  */
    F16SAMPLER2DRECTSHADOW = 481,  /* F16SAMPLER2DRECTSHADOW  */
    F16SAMPLERCUBESHADOW = 482,    /* F16SAMPLERCUBESHADOW  */
    F16SAMPLERCUBEARRAYSHADOW = 483, /* F16SAMPLERCUBEARRAYSHADOW  */
    IMAGE1D = 484,                 /* IMAGE1D  */
    IIMAGE1D = 485,                /* IIMAGE1D  */
    UIMAGE1D = 486,                /* UIMAGE1D  */
    IMAGE2D = 487,                 /* IMAGE2D  */
    IIMAGE2D = 488,                /* IIMAGE2D  */
    UIMAGE2D = 489,                /* UIMAGE2D  */
    IMAGE3D = 490,                 /* IMAGE3D  */
    IIMAGE3D = 491,                /* IIMAGE3D  */
    UIMAGE3D = 492,                /* UIMAGE3D  */
    IMAGE2DRECT = 493,             /* IMAGE2DRECT  */
    IIMAGE2DRECT = 494,            /* IIMAGE2DRECT  */
    UIMAGE2DRECT = 495,            /* UIMAGE2DRECT  */
    IMAGECUBE = 496,               /* IMAGECUBE  */
    IIMAGECUBE = 497,              /* IIMAGECUBE  */
    UIMAGECUBE = 498,              /* UIMAGECUBE  */
    IMAGEBUFFER = 499,             /* IMAGEBUFFER  */
    IIMAGEBUFFER = 500,            /* IIMAGEBUFFER  */
    UIMAGEBUFFER = 501,            /* UIMAGEBUFFER  */
    IMAGE1DARRAY = 502,            /* IMAGE1DARRAY  */
    IIMAGE1DARRAY = 503,           /* IIMAGE1DARRAY  */
    UIMAGE1DARRAY = 504,           /* UIMAGE1DARRAY  */
    IMAGE2DARRAY = 505,            /* IMAGE2DARRAY  */
    IIMAGE2DARRAY = 506,           /* IIMAGE2DARRAY  */
    UIMAGE2DARRAY = 507,           /* UIMAGE2DARRAY  */
    IMAGECUBEARRAY = 508,          /* IMAGECUBEARRAY  */
    IIMAGECUBEARRAY = 509,         /* IIMAGECUBEARRAY  */
    UIMAGECUBEARRAY = 510,         /* UIMAGECUBEARRAY  */
    IMAGE2DMS = 511,               /* IMAGE2DMS  */
    IIMAGE2DMS = 512,              /* IIMAGE2DMS  */
    UIMAGE2DMS = 513,              /* UIMAGE2DMS  */
    IMAGE2DMSARRAY = 514,          /* IMAGE2DMSARRAY  */
    IIMAGE2DMSARRAY = 515,         /* IIMAGE2DMSARRAY  */
    UIMAGE2DMSARRAY = 516,         /* UIMAGE2DMSARRAY  */
    F16IMAGE1D = 517,              /* F16IMAGE1D  */
    F16IMAGE2D = 518,              /* F16IMAGE2D  */
    F16IMAGE3D = 519,              /* F16IMAGE3D  */
    F16IMAGE2DRECT = 520,          /* F16IMAGE2DRECT  */
    F16IMAGECUBE = 521,            /* F16IMAGECUBE  */
    F16IMAGE1DARRAY = 522,         /* F16IMAGE1DARRAY  */
    F16IMAGE2DARRAY = 523,         /* F16IMAGE2DARRAY  */
    F16IMAGECUBEARRAY = 524,       /* F16IMAGECUBEARRAY  */
    F16IMAGEBUFFER = 525,          /* F16IMAGEBUFFER  */
    F16IMAGE2DMS = 526,            /* F16IMAGE2DMS  */
    F16IMAGE2DMSARRAY = 527,       /* F16IMAGE2DMSARRAY  */
    I64IMAGE1D = 528,              /* I64IMAGE1D  */
    U64IMAGE1D = 529,              /* U64IMAGE1D  */
    I64IMAGE2D = 530,              /* I64IMAGE2D  */
    U64IMAGE2D = 531,              /* U64IMAGE2D  */
    I64IMAGE3D = 532,              /* I64IMAGE3D  */
    U64IMAGE3D = 533,              /* U64IMAGE3D  */
    I64IMAGE2DRECT = 534,          /* I64IMAGE2DRECT  */
    U64IMAGE2DRECT = 535,          /* U64IMAGE2DRECT  */
    I64IMAGECUBE = 536,            /* I64IMAGECUBE  */
    U64IMAGECUBE = 537,            /* U64IMAGECUBE  */
    I64IMAGEBUFFER = 538,          /* I64IMAGEBUFFER  */
    U64IMAGEBUFFER = 539,          /* U64IMAGEBUFFER  */
    I64IMAGE1DARRAY = 540,         /* I64IMAGE1DARRAY  */
    U64IMAGE1DARRAY = 541,         /* U64IMAGE1DARRAY  */
    I64IMAGE2DARRAY = 542,         /* I64IMAGE2DARRAY  */
    U64IMAGE2DARRAY = 543,         /* U64IMAGE2DARRAY  */
    I64IMAGECUBEARRAY = 544,       /* I64IMAGECUBEARRAY  */
    U64IMAGECUBEARRAY = 545,       /* U64IMAGECUBEARRAY  */
    I64IMAGE2DMS = 546,            /* I64IMAGE2DMS  */
    U64IMAGE2DMS = 547,            /* U64IMAGE2DMS  */
    I64IMAGE2DMSARRAY = 548,       /* I64IMAGE2DMSARRAY  */
    U64IMAGE2DMSARRAY = 549,       /* U64IMAGE2DMSARRAY  */
    TEXTURECUBEARRAY = 550,        /* TEXTURECUBEARRAY  */
    ITEXTURECUBEARRAY = 551,       /* ITEXTURECUBEARRAY  */
    UTEXTURECUBEARRAY = 552,       /* UTEXTURECUBEARRAY  */
    TEXTURE1D = 553,               /* TEXTURE1D  */
    ITEXTURE1D = 554,              /* ITEXTURE1D  */
    UTEXTURE1D = 555,              /* UTEXTURE1D  */
    TEXTURE1DARRAY = 556,          /* TEXTURE1DARRAY  */
    ITEXTURE1DARRAY = 557,         /* ITEXTURE1DARRAY  */
    UTEXTURE1DARRAY = 558,         /* UTEXTURE1DARRAY  */
    TEXTURE2DRECT = 559,           /* TEXTURE2DRECT  */
    ITEXTURE2DRECT = 560,          /* ITEXTURE2DRECT  */
    UTEXTURE2DRECT = 561,          /* UTEXTURE2DRECT  */
    TEXTUREBUFFER = 562,           /* TEXTUREBUFFER  */
    ITEXTUREBUFFER = 563,          /* ITEXTUREBUFFER  */
    UTEXTUREBUFFER = 564,          /* UTEXTUREBUFFER  */
    TEXTURE2DMS = 565,             /* TEXTURE2DMS  */
    ITEXTURE2DMS = 566,            /* ITEXTURE2DMS  */
    UTEXTURE2DMS = 567,            /* UTEXTURE2DMS  */
    TEXTURE2DMSARRAY = 568,        /* TEXTURE2DMSARRAY  */
    ITEXTURE2DMSARRAY = 569,       /* ITEXTURE2DMSARRAY  */
    UTEXTURE2DMSARRAY = 570,       /* UTEXTURE2DMSARRAY  */
    F16TEXTURE1D = 571,            /* F16TEXTURE1D  */
    F16TEXTURE2D = 572,            /* F16TEXTURE2D  */
    F16TEXTURE3D = 573,            /* F16TEXTURE3D  */
    F16TEXTURE2DRECT = 574,        /* F16TEXTURE2DRECT  */
    F16TEXTURECUBE = 575,          /* F16TEXTURECUBE  */
    F16TEXTURE1DARRAY = 576,       /* F16TEXTURE1DARRAY  */
    F16TEXTURE2DARRAY = 577,       /* F16TEXTURE2DARRAY  */
    F16TEXTURECUBEARRAY = 578,     /* F16TEXTURECUBEARRAY  */
    F16TEXTUREBUFFER = 579,        /* F16TEXTUREBUFFER  */
    F16TEXTURE2DMS = 580,          /* F16TEXTURE2DMS  */
    F16TEXTURE2DMSARRAY = 581,     /* F16TEXTURE2DMSARRAY  */
    SUBPASSINPUT = 582,            /* SUBPASSINPUT  */
    SUBPASSINPUTMS = 583,          /* SUBPASSINPUTMS  */
    ISUBPASSINPUT = 584,           /* ISUBPASSINPUT  */
    ISUBPASSINPUTMS = 585,         /* ISUBPASSINPUTMS  */
    USUBPASSINPUT = 586,           /* USUBPASSINPUT  */
    USUBPASSINPUTMS = 587,         /* USUBPASSINPUTMS  */
    F16SUBPASSINPUT = 588,         /* F16SUBPASSINPUT  */
    F16SUBPASSINPUTMS = 589,       /* F16SUBPASSINPUTMS  */
    SPIRV_INSTRUCTION = 590,       /* SPIRV_INSTRUCTION  */
    SPIRV_EXECUTION_MODE = 591,    /* SPIRV_EXECUTION_MODE  */
    SPIRV_EXECUTION_MODE_ID = 592, /* SPIRV_EXECUTION_MODE_ID  */
    SPIRV_DECORATE = 593,          /* SPIRV_DECORATE  */
    SPIRV_DECORATE_ID = 594,       /* SPIRV_DECORATE_ID  */
    SPIRV_DECORATE_STRING = 595,   /* SPIRV_DECORATE_STRING  */
    SPIRV_TYPE = 596,              /* SPIRV_TYPE  */
    SPIRV_STORAGE_CLASS = 597,     /* SPIRV_STORAGE_CLASS  */
    SPIRV_BY_REFERENCE = 598,      /* SPIRV_BY_REFERENCE  */
    SPIRV_LITERAL = 599,           /* SPIRV_LITERAL  */
    ATTACHMENTEXT = 600,           /* ATTACHMENTEXT  */
    IATTACHMENTEXT = 601,          /* IATTACHMENTEXT  */
    UATTACHMENTEXT = 602,          /* UATTACHMENTEXT  */
    LEFT_OP = 603,                 /* LEFT_OP  */
    RIGHT_OP = 604,                /* RIGHT_OP  */
    INC_OP = 605,                  /* INC_OP  */
    DEC_OP = 606,                  /* DEC_OP  */
    LE_OP = 607,                   /* LE_OP  */
    GE_OP = 608,                   /* GE_OP  */
    EQ_OP = 609,                   /* EQ_OP  */
    NE_OP = 610,                   /* NE_OP  */
    AND_OP = 611,                  /* AND_OP  */
    OR_OP = 612,                   /* OR_OP  */
    XOR_OP = 613,                  /* XOR_OP  */
    MUL_ASSIGN = 614,              /* MUL_ASSIGN  */
    DIV_ASSIGN = 615,              /* DIV_ASSIGN  */
    ADD_ASSIGN = 616,              /* ADD_ASSIGN  */
    MOD_ASSIGN = 617,              /* MOD_ASSIGN  */
    LEFT_ASSIGN = 618,             /* LEFT_ASSIGN  */
    RIGHT_ASSIGN = 619,            /* RIGHT_ASSIGN  */
    AND_ASSIGN = 620,              /* AND_ASSIGN  */
    XOR_ASSIGN = 621,              /* XOR_ASSIGN  */
    OR_ASSIGN = 622,               /* OR_ASSIGN  */
    SUB_ASSIGN = 623,              /* SUB_ASSIGN  */
    STRING_LITERAL = 624,          /* STRING_LITERAL  */
    LEFT_PAREN = 625,              /* LEFT_PAREN  */
    RIGHT_PAREN = 626,             /* RIGHT_PAREN  */
    LEFT_BRACKET = 627,            /* LEFT_BRACKET  */
    RIGHT_BRACKET = 628,           /* RIGHT_BRACKET  */
    LEFT_BRACE = 629,              /* LEFT_BRACE  */
    RIGHT_BRACE = 630,             /* RIGHT_BRACE  */
    DOT = 631,                     /* DOT  */
    COMMA = 632,                   /* COMMA  */
    COLON = 633,                   /* COLON  */
    EQUAL = 634,                   /* EQUAL  */
    SEMICOLON = 635,               /* SEMICOLON  */
    BANG = 636,                    /* BANG  */
    DASH = 637,                    /* DASH  */
    TILDE = 638,                   /* TILDE  */
    PLUS = 639,                    /* PLUS  */
    STAR = 640,                    /* STAR  */
    SLASH = 641,                   /* SLASH  */
    PERCENT = 642,                 /* PERCENT  */
    LEFT_ANGLE = 643,              /* LEFT_ANGLE  */
    RIGHT_ANGLE = 644,             /* RIGHT_ANGLE  */
    VERTICAL_BAR = 645,            /* VERTICAL_BAR  */
    CARET = 646,                   /* CARET  */
    AMPERSAND = 647,               /* AMPERSAND  */
    QUESTION = 648,                /* QUESTION  */
    INVARIANT = 649,               /* INVARIANT  */
    HIGH_PRECISION = 650,          /* HIGH_PRECISION  */
    MEDIUM_PRECISION = 651,        /* MEDIUM_PRECISION  */
    LOW_PRECISION = 652,           /* LOW_PRECISION  */
    PRECISION = 653,               /* PRECISION  */
    PACKED = 654,                  /* PACKED  */
    RESOURCE = 655,                /* RESOURCE  */
    SUPERP = 656,                  /* SUPERP  */
    FLOATCONSTANT = 657,           /* FLOATCONSTANT  */
    INTCONSTANT = 658,             /* INTCONSTANT  */
    UINTCONSTANT = 659,            /* UINTCONSTANT  */
    BOOLCONSTANT = 660,            /* BOOLCONSTANT  */
    IDENTIFIER = 661,              /* IDENTIFIER  */
    TYPE_NAME = 662,               /* TYPE_NAME  */
    CENTROID = 663,                /* CENTROID  */
    IN = 664,                      /* IN  */
    OUT = 665,                     /* OUT  */
    INOUT = 666,                   /* INOUT  */
    STRUCT = 667,                  /* STRUCT  */
    VOID = 668,                    /* VOID  */
    WHILE = 669,                   /* WHILE  */
    BREAK = 670,                   /* BREAK  */
    CONTINUE = 671,                /* CONTINUE  */
    DO = 672,                      /* DO  */
    ELSE = 673,                    /* ELSE  */
    FOR = 674,                     /* FOR  */
    IF = 675,                      /* IF  */
    DISCARD = 676,                 /* DISCARD  */
    RETURN = 677,                  /* RETURN  */
    SWITCH = 678,                  /* SWITCH  */
    CASE = 679,                    /* CASE  */
    DEFAULT = 680,                 /* DEFAULT  */
    TERMINATE_INVOCATION = 681,    /* TERMINATE_INVOCATION  */
    TERMINATE_RAY = 682,           /* TERMINATE_RAY  */
    IGNORE_INTERSECTION = 683,     /* IGNORE_INTERSECTION  */
    UNIFORM = 684,                 /* UNIFORM  */
    SHARED = 685,                  /* SHARED  */
    BUFFER = 686,                  /* BUFFER  */
    TILEIMAGEEXT = 687,            /* TILEIMAGEEXT  */
    FLAT = 688,                    /* FLAT  */
    SMOOTH = 689,                  /* SMOOTH  */
    LAYOUT = 690,                  /* LAYOUT  */
    DOUBLECONSTANT = 691,          /* DOUBLECONSTANT  */
    INT16CONSTANT = 692,           /* INT16CONSTANT  */
    UINT16CONSTANT = 693,          /* UINT16CONSTANT  */
    FLOAT16CONSTANT = 694,         /* FLOAT16CONSTANT  */
    INT32CONSTANT = 695,           /* INT32CONSTANT  */
    UINT32CONSTANT = 696,          /* UINT32CONSTANT  */
    INT64CONSTANT = 697,           /* INT64CONSTANT  */
    UINT64CONSTANT = 698,          /* UINT64CONSTANT  */
    SUBROUTINE = 699,              /* SUBROUTINE  */
    DEMOTE = 700,                  /* DEMOTE  */
    FUNCTION = 701,                /* FUNCTION  */
    PAYLOADNV = 702,               /* PAYLOADNV  */
    PAYLOADINNV = 703,             /* PAYLOADINNV  */
    HITATTRNV = 704,               /* HITATTRNV  */
    CALLDATANV = 705,              /* CALLDATANV  */
    CALLDATAINNV = 706,            /* CALLDATAINNV  */
    PAYLOADEXT = 707,              /* PAYLOADEXT  */
    PAYLOADINEXT = 708,            /* PAYLOADINEXT  */
    HITATTREXT = 709,              /* HITATTREXT  */
    CALLDATAEXT = 710,             /* CALLDATAEXT  */
    CALLDATAINEXT = 711,           /* CALLDATAINEXT  */
    PATCH = 712,                   /* PATCH  */
    SAMPLE = 713,                  /* SAMPLE  */
    NONUNIFORM = 714,              /* NONUNIFORM  */
    COHERENT = 715,                /* COHERENT  */
    VOLATILE = 716,                /* VOLATILE  */
    RESTRICT = 717,                /* RESTRICT  */
    READONLY = 718,                /* READONLY  */
    WRITEONLY = 719,               /* WRITEONLY  */
    NONTEMPORAL = 720,             /* NONTEMPORAL  */
    DEVICECOHERENT = 721,          /* DEVICECOHERENT  */
    QUEUEFAMILYCOHERENT = 722,     /* QUEUEFAMILYCOHERENT  */
    WORKGROUPCOHERENT = 723,       /* WORKGROUPCOHERENT  */
    SUBGROUPCOHERENT = 724,        /* SUBGROUPCOHERENT  */
    NONPRIVATE = 725,              /* NONPRIVATE  */
    SHADERCALLCOHERENT = 726,      /* SHADERCALLCOHERENT  */
    NOPERSPECTIVE = 727,           /* NOPERSPECTIVE  */
    EXPLICITINTERPAMD = 728,       /* EXPLICITINTERPAMD  */
    PERVERTEXEXT = 729,            /* PERVERTEXEXT  */
    PERVERTEXNV = 730,             /* PERVERTEXNV  */
    PERPRIMITIVENV = 731,          /* PERPRIMITIVENV  */
    PERVIEWNV = 732,               /* PERVIEWNV  */
    PERTASKNV = 733,               /* PERTASKNV  */
    PERPRIMITIVEEXT = 734,         /* PERPRIMITIVEEXT  */
    TASKPAYLOADWORKGROUPEXT = 735, /* TASKPAYLOADWORKGROUPEXT  */
    PRECISE = 736                  /* PRECISE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 72 "MachineIndependent/glslang.y"

    struct {
        glslang::TSourceLoc loc;
        union {
            glslang::TString *string;
            int i;
            unsigned int u;
            long long i64;
            unsigned long long u64;
            bool b;
            double d;
        };
        glslang::TSymbol* symbol;
    } lex;
    struct {
        glslang::TSourceLoc loc;
        glslang::TOperator op;
        union {
            TIntermNode* intermNode;
            glslang::TIntermNodePair nodePair;
            glslang::TIntermTyped* intermTypedNode;
            glslang::TAttributes* attributes;
            glslang::TSpirvRequirement* spirvReq;
            glslang::TSpirvInstruction* spirvInst;
            glslang::TSpirvTypeParameters* spirvTypeParams;
        };
        union {
            glslang::TPublicType type;
            glslang::TFunction* function;
            glslang::TParameter param;
            glslang::TTypeLoc typeLine;
            glslang::TTypeList* typeList;
            glslang::TArraySizes* arraySizes;
            glslang::TIdentifierList* identifierList;
        };
        glslang::TTypeParameters* typeParameters;
    } interm;

#line 584 "MachineIndependent/glslang_tab.cpp.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (glslang::TParseContext* pParseContext);


#endif /* !YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED  */
