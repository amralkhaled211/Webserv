// #include "RequestHandler.hpp"
// #include "SendData.hpp"

// template<typename T>
// void		SendData::prepErrorResponse(int code, T& location)
// {
// 	std::string		errorPagePath = getErrorPagePath(code, location.getErrorPage());
// 	// std::cout << BOLD_RED << "errorPagePath: " << errorPagePath << RESET << "\n";
// 	std::string		contentType;
// 	int				fileStatus;

// 	std::cout << "IN PREP ERROR RESPONSE\n";

// 	if (!errorPagePath.empty())
// 	{ // need to create error response from errorPage
// 		// errorPagePath = location.getRoot() + location.getPrefix() + "/" + errorPagePath;
// 		removeExcessSlashes(errorPagePath);
// 		// std::cout << BOLD_RED << "errorPagePath: " << errorPagePath << RESET << "\n";

// 		std::cout << "ERROR PAGE PATH: " << errorPagePath << std::endl;

// 		fileStatus = readFromErrorPage(errorPagePath, _response.body); // this already reads into the body (passed by reference)
// 		if (fileStatus == SD_OK) { // body gets init with right error_page content
// 			contentType = mimeTypesMap_G[get_file_extension(errorPagePath)];

// 			std::cout << "CONTENT TYPE: -->" << contentType << "<--" << std::endl;

// 			if (contentType == "")
// 				createDfltResponseBody(code, contentType, "txt");
// 		}
// 		else if (fileStatus == SD_NO_READ_PERM) {
// 			if (code != 403) {
// 				prepErrorResponse(403, location);
// 				return ;
// 			}
// 			else
// 				createDfltResponseBody(code, contentType);
// 		}
// 		else if (fileStatus == SD_NO_FILE) {
// 			if (code != 404) {
// 				prepErrorResponse(404, location);
// 				return ;
// 			}
// 			else
// 				createDfltResponseBody(code, contentType);
// 		}
// 	}
// 	else // need to create hardcoded error response
// 		createDfltResponseBody(code, contentType);

// 	createResponseHeader(code, _response.body.size(), contentType);
// }