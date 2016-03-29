
@proc int m_vprintf( int str, char *format, va_list ap )
{
	int len;
	if( str ) m_clear(str); else str = m_create(10,1);
  	len = vsnprintf( m_buf(str), m_bufsize(str), format, ap );
  	if( len >= m_bufsize(str) ) {
	    	m_setlen(str, len+1 );
		len=vsnprintf( m_buf(str), m_bufsize(str), format, ap );
	}
	m_setlen(str, len );
	return str;
}

@proc int m_printf(int str, char *format, ... )
{
	va_list argptr;
	va_start(argptr, format);
	str = m_vprintf( str, format, argptr );
	va_end(argptr);
	return str;
}
