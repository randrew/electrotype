/* ============================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         http://code.google.com/p/freetype-gl/
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ============================================================================
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "font-manager.h"

// ------------------------------------------------------------ file_exists ---
int
file_exists( const char * filename )
{
    FILE * file = fopen( filename, "r" );
    if ( file )
    {
        fclose(file);
        return 1;
    }
    return 0;
}

// ----------------------------------------------------------------- wcsdup ---
wchar_t *
wcsdup( const wchar_t *string )
{
    wchar_t * result;
    assert( string );
    result = (wchar_t *) malloc( (wcslen(string) + 1) * sizeof(wchar_t) );
    wcscpy( result, string );
    return result;
}


// ------------------------------------------------------- font_manager_new ---
font_manager_t *
font_manager_new( size_t width, size_t height, size_t depth )
{
    font_manager_t *self;
    texture_atlas_t *atlas = texture_atlas_new( width, height, depth );
    self = (font_manager_t *) malloc( sizeof(font_manager_t) );
    assert(self);
    self->atlas = atlas;
    self->fonts = vector_new( sizeof(texture_font_t *) );
    self->cache = wcsdup( L" " );
    return self;
}


// ---------------------------------------------------- font_manager_delete ---
void
font_manager_delete( font_manager_t * self )
{
    assert( self );

    size_t i;
    texture_font_t *font;
    for( i=0; i<vector_size( self->fonts ); ++i)
    {
        font = *(texture_font_t **) vector_get( self->fonts, i );
        texture_font_delete( font );
    }
    vector_delete( self->fonts );
    texture_atlas_delete( self->atlas );
    if( self->cache )
    {
        free( self->cache );
    }
    free( self );
}



// ----------------------------------------------- font_manager_delete_font ---
void
font_manager_delete_font( font_manager_t * self,
                          texture_font_t * font)
{
    size_t i;
    assert( self );
    assert( font );

    texture_font_t *other;
    for( i=0; i<self->fonts->size;++i )
    {
        other = (texture_font_t *) vector_get( self->fonts, i );
        if ( (strcmp(font->filename, other->filename) == 0)
               && ( font->size == other->size) )
        {
            vector_erase( self->fonts, i);
            break;
        }
    }
    texture_font_delete( font );
}



// ----------------------------------------- font_manager_get_from_filename ---
texture_font_t *
font_manager_get_from_filename( font_manager_t *self,
                                const char * filename,
                                const float size )
{
    size_t i;
    texture_font_t *font;

    assert( self );
    for( i=0; i<vector_size(self->fonts); ++i )
    {
        font = * (texture_font_t **) vector_get( self->fonts, i );
        if( (strcmp(font->filename, filename) == 0) && ( font->size == size) )
        {
            return font;
        }
    }
    font = texture_font_new( self->atlas, filename, size );
    if( font )
    {
        vector_push_back( self->fonts, &font );
        texture_font_load_glyphs( font, self->cache );
        return font;
    }
    fprintf( stderr, "Unable to load \"%s\" (size=%.1f)\n", filename, size );
    return 0;
}

texture_font_t *
font_manager_get_from_desc( font_manager_t *self, const font_desc_t *desc )
{
    assert( self );


    if( file_exists( desc->family ) ) {
        return font_manager_get_from_filename( self, desc->family, desc->size );
    }

    char *filename = font_desc_find_filename( desc );
    if( !filename ) return 0;

    texture_font_t *font =
        font_manager_get_from_filename( self, filename, desc->size );

    free (filename);

    return font;
}
