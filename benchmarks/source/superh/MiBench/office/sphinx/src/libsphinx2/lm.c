/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */

/*
 * lm.c -- Interpolation of various language models.
 *
 * HISTORY
 * 
 * 01-Apr-97	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Started, based on earlier FBS6 version.
 */


/* Currently, interpolation of dynamic cache LM and static trigram LM */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "s2types.h"
#include "CM_macros.h"
#include "basic_types.h"
#include "list.h"
#include "hash.h"
#include "lmclass.h"
#include "lm_3g.h"
#include "lm.h"
#include "cache_lm.h"
#include "search_const.h"
#include "msd.h"
#include "dict.h"
#include "kb.h"
#include "err.h"
#include "log.h"


static cache_lm_t *clm = NULL;


int32 lm_tg_score (int32 w1, int32 w2, int32 w3)
{
    int32 cscr, tscr, remwt;
    lm_t *lm3g;
    
    if (! clm)
	return (lm3g_tg_score (w1, w2, w3));
    
    lm3g = lm_get_current ();

    /* Get cache LM score and apply language weight */
    cscr = cache_lm_score (clm, w2, w3, &remwt);
    cscr *= lm3g->lw;

    /* Get static trigram LM score, apply remaining weight */
    tscr = lm3g_tg_score (w1, w2, w3);
    tscr += remwt * lm3g->lw;
    
    /* Return MAX of static trigram LM and dynamic cache LM scores (approx to sum) */
    return (cscr > tscr) ? cscr : tscr;
}


int32 lm_bg_score (int32 w1, int32 w2)
{
    int32 cscr, tscr, remwt;
    lm_t *lm3g;
    
    if (! clm)
	return (lm3g_bg_score (w1, w2));
    
    lm3g = lm_get_current ();

    /* Get cache LM score and apply language weight */
    cscr = cache_lm_score (clm, w1, w2, &remwt);
    cscr *= lm3g->lw;

    /* Get static trigram LM score, apply remaining weight */
    tscr = lm3g_bg_score (w1, w2);
    tscr += remwt * lm3g->lw;
    
    /* Return MAX of static trigram LM and dynamic cache LM scores (approx to sum) */
    return (cscr > tscr) ? cscr : tscr;
}


int32 lm_ug_score (int32 w)
{
    return (lm3g_ug_score (w));
}


void lm_cache_lm_init ( void )
{
    if (clm)
	return;
    
    /* Hack!!  Hardwired parameters to cache_lm_init */
    clm = cache_lm_init (0.0001, 0.001, 0.04, 100, 0.07);
}


void lm_cache_lm_add_ug (int32 w)
{
    int32 ugscr;
    lm_t *lm3g;

    if (! clm)
	return;

    lm3g = lm_get_current ();
    ugscr = lm3g_ug_score (w) * lm3g->invlw;
    if (ugscr >= clm->ugprob_thresh)
	return;
#if 0
    E_INFO("Adding unigram %s (scr %d, thresh %d) to cache LM\n",
	   kb_get_word_str(w), ugscr, clm->ugprob_thresh);
#endif
    cache_lm_add_ug (clm, w);
}


void lm_cache_lm_add_bg (int32 w1, int32 w2)
{
    if (! clm)
	return;
#if 0
    E_INFO("Adding bigram %s,%s to cache LM\n",
	   kb_get_word_str (w1), kb_get_word_str (w2));
#endif
    cache_lm_add_bg (clm, w1, w2);
}


void lm_cache_lm_dump (char *file)
{
    if (! clm)
	return;
    cache_lm_dump (clm, file);
}


void lm_cache_lm_load (char *file)
{
    if (! clm)
	return;
    cache_lm_load (clm, file);
}
