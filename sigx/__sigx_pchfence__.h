// this file just exists as a fence for the precompiled headers;
// with MSVC it doesn't even need to exist when pch is turned on, but as I 
// include it unconditionally it has to be present.
// 
// I don't include any files here as 1) I don't want to impose any includes on 
// each cpp file and 2) the compilation will work without side effects 
// (performance etc...) if precompiled headers are turned off.
// Rather __sigx_pchfence__.cpp includes everything that sigx needs and each 
// cpp file includes its own more accurate headers which also will be looked 
// up in the precomiled header file sigx-2.0.pch;
