#include "jcmarker.h"

void jinit_marker_writer ( sJpeg_compress_struct *cinfo )
{
	my_marker_writer *marker;

  /* Create the subobject */
  marker = ( my_marker_writer * )(*cinfo->mem->alloc_small) ( ( sJpeg_common_struct *) cinfo, JPOOL_IMAGE, SIZEOF( my_marker_writer ) );
  cinfo->marker = &marker->pub;
  /* Initialize method pointers */
  marker->pub.write_file_header = write_file_header;
  marker->pub.write_frame_header = write_frame_header;
  marker->pub.write_scan_header = write_scan_header;
  marker->pub.write_file_trailer = write_file_trailer;
  marker->pub.write_tables_only = write_tables_only;
  marker->pub.write_marker_header = write_marker_header;
  marker->pub.write_marker_byte = write_marker_byte;
  /* Initialize private state */
  marker->last_restart_interval = 0;
}

static void write_scan_header ( sJpeg_compress_struct *cinfo )
{
	my_marker_writer *marker = ( my_marker_writer * ) cinfo->marker;
  int i;
  sJpeg_component_info *compptr;

  if (cinfo->arith_code) 
	{
    /* Emit arith conditioning info.  We may have some duplication
     * if the file has multiple scans, but it's so small it's hardly
     * worth worrying about.
     */
    emit_dac(cinfo);
  } 
	else 
	{
    /* Emit Huffman tables.
     * Note that emit_dht() suppresses any duplicate tables.
     */
    for (i = 0; i < cinfo->comps_in_scan; i++) 
		{
      compptr = cinfo->cur_comp_info[i];
      /* DC needs no table for refinement scan */
      if (cinfo->Ss == 0 && cinfo->Ah == 0)
				emit_dht(cinfo, compptr->dc_tbl_no, FALSE);
      /* AC needs no table when not present */
      if (cinfo->Se)
				emit_dht(cinfo, compptr->ac_tbl_no, TRUE);
    }
  }

  /* Emit DRI if required --- note that DRI value could change for each scan.
   * We avoid wasting space with unnecessary DRIs, however.
   */
  if (cinfo->restart_interval != marker->last_restart_interval) 
	{
    emit_dri(cinfo);
    marker->last_restart_interval = cinfo->restart_interval;
  }

  emit_sos(cinfo);
}

static void write_marker_header ( sJpeg_compress_struct *cinfo, int marker, unsigned int datalen )
{
	if (datalen > (unsigned int) 65533)		/* safety check */
    ERREXIT(cinfo, JERR_BAD_LENGTH);
  emit_marker(cinfo, (JPEG_MARKER) marker);
  emit_2bytes(cinfo, (int) (datalen + 2));	/* total length */
}

static void write_marker_byte ( sJpeg_compress_struct *cinfo, int val )
{
	emit_byte(cinfo, val);
}

static void write_file_trailer ( sJpeg_compress_struct *cinfo )
{
	 emit_marker(cinfo, M_EOI);
}

static void write_tables_only ( sJpeg_compress_struct *cinfo )
{
	int i;

  emit_marker(cinfo, M_SOI);

  for (i = 0; i < NUM_QUANT_TBLS; i++) 
	{
    if (cinfo->quant_tbl_ptrs[i] != NULL)
      (void) emit_dqt(cinfo, i);
  }

  if (! cinfo->arith_code) 
	{
    for (i = 0; i < NUM_HUFF_TBLS; i++) 
		{
      if (cinfo->dc_huff_tbl_ptrs[i] != NULL)
				emit_dht(cinfo, i, FALSE);
      if (cinfo->ac_huff_tbl_ptrs[i] != NULL)
				emit_dht(cinfo, i, TRUE);
    }
  }

  emit_marker(cinfo, M_EOI);
}

static void write_frame_header ( sJpeg_compress_struct *cinfo )
{
	int ci, prec;
  boolean is_baseline;
  sJpeg_component_info *compptr;
  
  /* Emit DQT for each quantization table.
   * Note that emit_dqt() suppresses any duplicate tables.
   */
  prec = 0;
  for ( ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components; ci++, compptr++ ) 
	{
    prec += emit_dqt(cinfo, compptr->quant_tbl_no);
  }
  /* now prec is nonzero iff there are any 16-bit quant tables. */

  /* Check for a non-baseline specification.
   * Note we assume that Huffman table numbers won't be changed later.
   */
  if (cinfo->arith_code || cinfo->progressive_mode ||
      cinfo->data_precision != 8 || cinfo->block_size != DCTSIZE) {
    is_baseline = FALSE;
  } else {
    is_baseline = TRUE;
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      if (compptr->dc_tbl_no > 1 || compptr->ac_tbl_no > 1)
	is_baseline = FALSE;
    }
    if (prec && is_baseline) {
      is_baseline = FALSE;
      /* If it's baseline except for quantizer size, warn the user */
      TRACEMS(cinfo, 0, JTRC_16BIT_TABLES);
    }
  }

  /* Emit the proper SOF marker */
  if (cinfo->arith_code) 
	{
    if (cinfo->progressive_mode)
      emit_sof(cinfo, M_SOF10); /* SOF code for progressive arithmetic */
    else
      emit_sof(cinfo, M_SOF9);  /* SOF code for sequential arithmetic */
  } 
	else 
	{
    if (cinfo->progressive_mode)
      emit_sof(cinfo, M_SOF2);	/* SOF code for progressive Huffman */
    else if (is_baseline)
      emit_sof(cinfo, M_SOF0);	/* SOF code for baseline implementation */
    else
      emit_sof(cinfo, M_SOF1);	/* SOF code for non-baseline Huffman file */
  }

  /* Check to emit LSE inverse color transform specification marker */
  if (cinfo->color_transform)
    emit_lse_ict(cinfo);

  /* Check to emit pseudo SOS marker */
  if (cinfo->progressive_mode && cinfo->block_size != DCTSIZE)
    emit_pseudo_sos(cinfo);
}

static void write_file_header ( sJpeg_compress_struct *cinfo )
{
	my_marker_writer *marker = (my_marker_writer *) cinfo->marker;

  emit_marker(cinfo, M_SOI);	/* first the SOI */

  /* SOI is defined to reset restart interval to 0 */
  marker->last_restart_interval = 0;

  if (cinfo->write_JFIF_header)	/* next an optional JFIF APP0 */
    emit_jfif_app0(cinfo);
  if (cinfo->write_Adobe_marker) /* next an optional Adobe APP14 */
    emit_adobe_app14(cinfo);
}

static void emit_byte ( sJpeg_compress_struct *cinfo, int val )
{
	struct sJpeg_destination_mgr * dest = cinfo->dest;

  *(dest->next_output_byte)++ = (JOCTET) val;
  if (--dest->free_in_buffer == 0) {
    if (! (*dest->empty_output_buffer) (cinfo))
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
  }
}

static void emit_marker ( sJpeg_compress_struct *cinfo, JPEG_MARKER mark )
{
	emit_byte(cinfo, 0xFF);
  emit_byte(cinfo, (int) mark);
}

static void emit_2bytes ( sJpeg_compress_struct *cinfo, int value )
{
	emit_byte(cinfo, (value >> 8) & 0xFF);
  emit_byte(cinfo, value & 0xFF);
}

static int emit_dqt ( sJpeg_compress_struct *cinfo, int index )
{
	JQUANT_TBL * qtbl = cinfo->quant_tbl_ptrs[index];
  int prec;
  int i;

  if (qtbl == NULL)
    ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, index);

  prec = 0;
  for (i = 0; i <= cinfo->lim_Se; i++) {
    if (qtbl->quantval[cinfo->natural_order[i]] > 255)
      prec = 1;
  }

  if (! qtbl->sent_table) 
	{
    emit_marker( cinfo, M_DQT );

    emit_2bytes( cinfo, prec ? cinfo->lim_Se * 2 + 2 + 1 + 2 : cinfo->lim_Se + 1 + 1 + 2 );

    emit_byte(cinfo, index + (prec<<4));

    for (i = 0; i <= cinfo->lim_Se; i++) 
		{
      /* The table entries must be emitted in zigzag order. */
      unsigned int qval = qtbl->quantval[cinfo->natural_order[i]];
      if (prec)
				emit_byte( cinfo, (int) (qval >> 8) );
      emit_byte( cinfo, (int) (qval & 0xFF) );
    }

    qtbl->sent_table = TRUE;
  }

  return prec;
}

static void emit_dht ( sJpeg_compress_struct *cinfo, int index, boolean is_ac )
{
	JHUFF_TBL * htbl;
  int length, i;
  
  if (is_ac) {
    htbl = cinfo->ac_huff_tbl_ptrs[index];
    index += 0x10;		/* output index has AC bit set */
  } else {
    htbl = cinfo->dc_huff_tbl_ptrs[index];
  }

  if (htbl == NULL)
    ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, index);
  
  if (! htbl->sent_table) {
    emit_marker(cinfo, M_DHT);
    
    length = 0;
    for (i = 1; i <= 16; i++)
      length += htbl->bits[i];
    
    emit_2bytes(cinfo, length + 2 + 1 + 16);
    emit_byte(cinfo, index);
    
    for (i = 1; i <= 16; i++)
      emit_byte(cinfo, htbl->bits[i]);
    
    for (i = 0; i < length; i++)
      emit_byte(cinfo, htbl->huffval[i]);
    
    htbl->sent_table = TRUE;
  }
}

static void emit_adobe_app14 ( sJpeg_compress_struct *cinfo )
{
	/*
   * Length of APP14 block	(2 bytes)
   * Block ID			(5 bytes - ASCII "Adobe")
   * Version Number		(2 bytes - currently 100)
   * Flags0			(2 bytes - currently 0)
   * Flags1			(2 bytes - currently 0)
   * Color transform		(1 byte)
   *
   * Although Adobe TN 5116 mentions Version = 101, all the Adobe files
   * now in circulation seem to use Version = 100, so that's what we write.
   *
   * We write the color transform byte as 1 if the JPEG color space is
   * YCbCr, 2 if it's YCCK, 0 otherwise.  Adobe's definition has to do with
   * whether the encoder performed a transformation, which is pretty useless.
   */
  
  emit_marker(cinfo, M_APP14);
  
  emit_2bytes(cinfo, 2 + 5 + 2 + 2 + 2 + 1); /* length */

  emit_byte(cinfo, 0x41);	/* Identifier: ASCII "Adobe" */
  emit_byte(cinfo, 0x64);
  emit_byte(cinfo, 0x6F);
  emit_byte(cinfo, 0x62);
  emit_byte(cinfo, 0x65);
  emit_2bytes(cinfo, 100);	/* Version */
  emit_2bytes(cinfo, 0);	/* Flags0 */
  emit_2bytes(cinfo, 0);	/* Flags1 */
  switch (cinfo->jpeg_color_space) 
	{
  case JCS_YCbCr:
    emit_byte(cinfo, 1);	/* Color transform = 1 */
    break;
  case JCS_YCCK:
    emit_byte(cinfo, 2);	/* Color transform = 2 */
    break;
  default:
    emit_byte(cinfo, 0);	/* Color transform = 0 */
    break;
  }
}

static void emit_dac ( sJpeg_compress_struct *cinfo )
{
#ifdef C_ARITH_CODING_SUPPORTED
  char dc_in_use[NUM_ARITH_TBLS];
  char ac_in_use[NUM_ARITH_TBLS];
  int length, i;
  sJpeg_component_info *compptr;

  for (i = 0; i < NUM_ARITH_TBLS; i++)
    dc_in_use[i] = ac_in_use[i] = 0;

  for (i = 0; i < cinfo->comps_in_scan; i++) 
	{
    compptr = cinfo->cur_comp_info[i];
    /* DC needs no table for refinement scan */
    if (cinfo->Ss == 0 && cinfo->Ah == 0)
      dc_in_use[compptr->dc_tbl_no] = 1;
    /* AC needs no table when not present */
    if (cinfo->Se)
      ac_in_use[compptr->ac_tbl_no] = 1;
  }

  length = 0;
  for (i = 0; i < NUM_ARITH_TBLS; i++)
    length += dc_in_use[i] + ac_in_use[i];

  if (length) {
    emit_marker(cinfo, M_DAC);

    emit_2bytes(cinfo, length*2 + 2);

    for (i = 0; i < NUM_ARITH_TBLS; i++) 
		{
      if (dc_in_use[i]) 
			{
				emit_byte(cinfo, i);
				emit_byte(cinfo, cinfo->arith_dc_L[i] + (cinfo->arith_dc_U[i]<<4));
      }
      if (ac_in_use[i]) 
			{
				emit_byte(cinfo, i + 0x10);
				emit_byte(cinfo, cinfo->arith_ac_K[i]);
      }
    }
  }
#endif /* C_ARITH_CODING_SUPPORTED */
}

static void emit_dri ( sJpeg_compress_struct *cinfo )
{
	emit_marker(cinfo, M_DRI);
  emit_2bytes(cinfo, 4);	/* fixed length */
  emit_2bytes(cinfo, (int) cinfo->restart_interval);
}

static void emit_lse_ict ( sJpeg_compress_struct *cinfo )
{
	/* Support only 1 transform */
  if (cinfo->color_transform != JCT_SUBTRACT_GREEN ||
      cinfo->num_components < 3)
    ERREXIT(cinfo, JERR_CONVERSION_NOTIMPL);

  emit_marker(cinfo, M_JPG8);
  
  emit_2bytes(cinfo, 24);	/* fixed length */

  emit_byte(cinfo, 0x0D);	/* ID inverse transform specification */
  emit_2bytes(cinfo, MAXJSAMPLE);	/* MAXTRANS */
  emit_byte(cinfo, 3);		/* Nt=3 */
  emit_byte(cinfo, cinfo->comp_info[1].component_id);
  emit_byte(cinfo, cinfo->comp_info[0].component_id);
  emit_byte(cinfo, cinfo->comp_info[2].component_id);
  emit_byte(cinfo, 0x80);	/* F1: CENTER1=1, NORM1=0 */
  emit_2bytes(cinfo, 0);	/* A(1,1)=0 */
  emit_2bytes(cinfo, 0);	/* A(1,2)=0 */
  emit_byte(cinfo, 0);		/* F2: CENTER2=0, NORM2=0 */
  emit_2bytes(cinfo, 1);	/* A(2,1)=1 */
  emit_2bytes(cinfo, 0);	/* A(2,2)=0 */
  emit_byte(cinfo, 0);		/* F3: CENTER3=0, NORM3=0 */
  emit_2bytes(cinfo, 1);	/* A(3,1)=1 */
  emit_2bytes(cinfo, 0);	/* A(3,2)=0 */
}

static void emit_jfif_app0 ( sJpeg_compress_struct *cinfo )
{
	/*
   * Length of APP0 block	(2 bytes)
   * Block ID			(4 bytes - ASCII "JFIF")
   * Zero byte			(1 byte to terminate the ID string)
   * Version Major, Minor	(2 bytes - major first)
   * Units			(1 byte - 0x00 = none, 0x01 = inch, 0x02 = cm)
   * Xdpu			(2 bytes - dots per unit horizontal)
   * Ydpu			(2 bytes - dots per unit vertical)
   * Thumbnail X size		(1 byte)
   * Thumbnail Y size		(1 byte)
   */
  
  emit_marker(cinfo, M_APP0);
  
  emit_2bytes(cinfo, 2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1); /* length */

  emit_byte(cinfo, 0x4A);	/* Identifier: ASCII "JFIF" */
  emit_byte(cinfo, 0x46);
  emit_byte(cinfo, 0x49);
  emit_byte(cinfo, 0x46);
  emit_byte(cinfo, 0);
  emit_byte(cinfo, cinfo->JFIF_major_version); /* Version fields */
  emit_byte(cinfo, cinfo->JFIF_minor_version);
  emit_byte(cinfo, cinfo->density_unit); /* Pixel size information */
  emit_2bytes(cinfo, (int) cinfo->X_density);
  emit_2bytes(cinfo, (int) cinfo->Y_density);
  emit_byte(cinfo, 0);		/* No thumbnail image */
  emit_byte(cinfo, 0);
}

static void emit_pseudo_sos ( sJpeg_compress_struct *cinfo )
{
	emit_marker(cinfo, M_SOS);
  
  emit_2bytes(cinfo, 2 + 1 + 3); /* length */
  
  emit_byte(cinfo, 0); /* Ns */

  emit_byte(cinfo, 0); /* Ss */
  emit_byte(cinfo, cinfo->block_size * cinfo->block_size - 1); /* Se */
  emit_byte(cinfo, 0); /* Ah/Al */
}

static void emit_sof ( sJpeg_compress_struct *cinfo, JPEG_MARKER code )
{
	int ci;
  sJpeg_component_info *compptr;
  
  emit_marker(cinfo, code);
  emit_2bytes(cinfo, 3 * cinfo->num_components + 2 + 5 + 1); /* length */

  /* Make sure image isn't bigger than SOF field can handle */
  if ((long) cinfo->jpeg_height > 65535L || (long) cinfo->jpeg_width > 65535L)
    ERREXIT1(cinfo, JERR_IMAGE_TOO_BIG, (unsigned int) 65535);

  emit_byte(cinfo, cinfo->data_precision);
  emit_2bytes(cinfo, (int) cinfo->jpeg_height);
  emit_2bytes(cinfo, (int) cinfo->jpeg_width);

  emit_byte(cinfo, cinfo->num_components);

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components; ci++, compptr++) 
	{
    emit_byte(cinfo, compptr->component_id);
    emit_byte(cinfo, (compptr->h_samp_factor << 4) + compptr->v_samp_factor);
    emit_byte(cinfo, compptr->quant_tbl_no);
  }
}

static void emit_sos ( sJpeg_compress_struct *cinfo )
{
	int i, td, ta;
  sJpeg_component_info *compptr;
  
  emit_marker(cinfo, M_SOS);
  
  emit_2bytes(cinfo, 2 * cinfo->comps_in_scan + 2 + 1 + 3); /* length */
  
  emit_byte(cinfo, cinfo->comps_in_scan);
  
  for (i = 0; i < cinfo->comps_in_scan; i++) {
    compptr = cinfo->cur_comp_info[i];
    emit_byte(cinfo, compptr->component_id);

    /* We emit 0 for unused field(s); this is recommended by the P&M text
     * but does not seem to be specified in the standard.
     */

    /* DC needs no table for refinement scan */
    td = cinfo->Ss == 0 && cinfo->Ah == 0 ? compptr->dc_tbl_no : 0;
    /* AC needs no table when not present */
    ta = cinfo->Se ? compptr->ac_tbl_no : 0;

    emit_byte(cinfo, (td << 4) + ta);
  }

  emit_byte(cinfo, cinfo->Ss);
  emit_byte(cinfo, cinfo->Se);
  emit_byte(cinfo, (cinfo->Ah << 4) + cinfo->Al);
}