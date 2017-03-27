#include "dbHelper.h"
#include "utils.h"

database::dbHelper::dbHelper(utility::string_t ConnectionString) : _connected(false)
{
	pwszConnStr = new SQLWCHAR[wcslen(ConnectionString.c_str()) + 1];
	wcscpy_s(pwszConnStr, wcslen(ConnectionString.c_str()) + 1, ConnectionString.c_str());
}

bool database::dbHelper::isConnected()
{
	return _connected;
}

void database::dbHelper::Disconnect()
{
	if (_connected)
	{
		SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
		SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
		_connected = false;
	}
}

void database::dbHelper::Connect()
{
	SQLWCHAR OutConnStr[255];
	SQLSMALLINT OutConnStrLen;
	RETCODE rc;

	if (!isConnected())
	{
		rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &this->hEnv);

		if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
		{
			rc = SQLSetEnvAttr(this->hEnv, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);

			if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
			{
				rc = SQLAllocHandle(SQL_HANDLE_DBC, this->hEnv, &this->hDbc);

				if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
				{
					rc = SQLSetConnectAttr(this->hDbc, SQL_LOGIN_TIMEOUT, reinterpret_cast<SQLPOINTER>(5), 0);
					rc = SQLDriverConnect(this->hDbc, nullptr, pwszConnStr, SQL_NTS, OutConnStr, 255, &OutConnStrLen, SQL_DRIVER_COMPLETE);
					this->_connected = rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;
				}
			}
		}
	}
}

void  database::dbHelper::setConnectionString(utility::string_t ConnectionString)
{
	
	delete[] pwszConnStr;

	pwszConnStr = new SQLWCHAR[wcslen(ConnectionString.c_str()) + 1];
	wcscpy_s(pwszConnStr, wcslen(ConnectionString.c_str()) + 1, ConnectionString.c_str());
}

bool database::dbHelper::executeQuery(const SQLWCHAR *QueryString, helperParameters Parameters[], int parameterCount, web::json::value &Result)
{
	
	if (!isConnected()) return false;
	
	SQLHSTMT hStmt;
	RETCODE rc;
	SQLSMALLINT	   numCols;

	web::json::value headers;
	web::json::value Rows;

	SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

	for (int i = 0; i < parameterCount; i++)
		SQLBindParameter(hStmt, i + 1, Parameters[i].fParamType, Parameters[i].fCType, Parameters[i].fSqlType, Parameters[i].cbColDef, Parameters[i].ibScale, Parameters[i].rgbValue, Parameters[i].cbValueMax, Parameters[i].pcbValue);

	rc = SQLPrepare(hStmt, const_cast<wchar_t*>(QueryString), SQL_NTS);

	rc = SQLExecute(hStmt);

	if (!(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO || rc == SQL_NO_DATA))
	{
		SQLWCHAR sqlState[8], msgText[1024];
		SQLINTEGER NativeErrorPtr;
		SQLSMALLINT TextLengthPtr;
		SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, 1, sqlState,
			&NativeErrorPtr, msgText, sizeof(msgText), &TextLengthPtr);
		SQLFreeStmt(hStmt, SQL_DROP);

		std::wcout << QueryString << std::endl;
		std::wcout << "Error  " << std::endl << msgText << std::endl;
		
		
		return false;
	}

	rc = SQLNumResultCols(hStmt, &numCols);

	SQLWCHAR **      ColumnName = new SQLWCHAR *[numCols];
	SQLSMALLINT *   ColumnNameLen = new SQLSMALLINT[numCols];
	SQLSMALLINT *   ColumnDataType = new SQLSMALLINT[numCols];
	SQLULEN *       ColumnDataSize = new SQLULEN[numCols];
	SQLSMALLINT *    ColumnDataDigits = new SQLSMALLINT[numCols];
	SQLSMALLINT *   ColumnDataNullable = new SQLSMALLINT[numCols];
	SQLCHAR **      ColumnData = new SQLCHAR *[numCols];
	SQLLEN *        ColumnDataLen = new SQLLEN[numCols];

	for (int i = 0; i<numCols; i++) {
		ColumnName[i] = (SQLWCHAR *)malloc(MAX_COL_NAME_LEN);
		rc = SQLDescribeCol(
			hStmt,                    // Select Statement (Prepared)
			i + 1,                      // Columnn Number
			ColumnName[i],            // Column Name (returned)
			MAX_COL_NAME_LEN,         // size of Column Name buffer
			&ColumnNameLen[i],        // Actual size of column name
			&ColumnDataType[i],       // SQL Data type of column
			&ColumnDataSize[i],       // Data size of column in table
			&ColumnDataDigits[i],     // Number of decimal digits
			&ColumnDataNullable[i]);  // Whether column nullable

									  // Display column data
		printf("\nColumn : %i\n", i + 1);
		printf("Column Name : %s\n  Column Name Len : %i\n  SQL Data Type : %i\n  Data Size : %i\n  DecimalDigits : %i\n  Nullable %i\n",
			ColumnName[i], (int)ColumnNameLen[i], (int)ColumnDataType[i],
			(int)ColumnDataSize[i], (int)ColumnDataDigits[i],
			(int)ColumnDataNullable[i]);

		// Bind column, changing SQL data type to C data type
		// (assumes INT and VARCHAR for now)
		
		switch (ColumnDataType[i]) {
		case SQL_INTEGER:
			ColumnDataType[i] = SQL_C_LONG;
			break;
		case SQL_VARCHAR:
			ColumnDataType[i] = SQL_C_WCHAR;
			ColumnDataSize[i] = ColumnDataSize[i] * sizeof(wchar_t);
			break;
		}

		ColumnData[i] = (SQLCHAR *)malloc(ColumnDataSize[i] + 1);

		rc = SQLBindCol(hStmt,                  // Statement handle
			i + 1,                    // Column number
			ColumnDataType[i],      // C Data Type
			ColumnData[i],          // Data buffer
			ColumnDataSize[i],      // Size of Data Buffer
			&ColumnDataLen[i]); // Size of data returned

		headers[i] = web::json::value(ColumnName[i]);
	}

	Result[LHEADER] = headers;

	int rowCount = 0;

	while (numCols > 0 && SQLFetch(hStmt) != SQL_NO_DATA)
	{
		web::json::value newRow;

		for (int c = 0; c < numCols; c++)
		{
			if (ColumnDataType[c] == SQL_C_WCHAR)
				newRow[c] = web::json::value(reinterpret_cast<wchar_t *>(ColumnData[c]) );
			else if (ColumnDataType[c] == SQL_C_CHAR || ColumnDataType[c] == SQL_LONGVARCHAR )
				newRow[c] = web::json::value(chartoWString(reinterpret_cast<char *>(ColumnData[c])));
			else if (ColumnDataType[c] == SQL_FLOAT)
				newRow[c] = web::json::value(*reinterpret_cast<float *>(ColumnData[c]));
			else if (ColumnDataType[c] == SQL_C_LONG)
				newRow[c] = web::json::value(*reinterpret_cast<int *>(ColumnData[c]));
			else if (ColumnDataType[c] == SQL_C_TIMESTAMP)
				newRow[c] = web::json::value( reinterpret_cast<utility::datetime *>(ColumnData[c])->to_string() );
		}
		Rows[rowCount++] = newRow;
	}

	Result[LDATA] = Rows;

	while (SQL_NO_DATA_FOUND != SQLMoreResults(hStmt));
	
	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

	delete [] ColumnName;
	delete [] ColumnNameLen;
	delete [] ColumnDataType;
	delete [] ColumnDataSize;
	delete [] ColumnDataDigits;
	delete [] ColumnDataNullable;
	delete [] ColumnData;
	delete [] ColumnDataLen;

	return true;
}

database::dbHelper::~dbHelper()
{
	Disconnect();
	delete[] pwszConnStr;
}