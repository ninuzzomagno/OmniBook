#include"ClassDocument.h"

using json = nlohmann::json;

std::string url_encode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }
    return escaped.str();
}

Document::~Document(){
	if (this->pix) fz_drop_pixmap(this->ctx, this->pix);
    if (this->stext_page) fz_drop_stext_page(this->ctx, this->stext_page);
    if (this->doc) fz_drop_document(this->ctx, this->doc);
    if (this->ctx) fz_drop_context(this->ctx);
    if (this->texture) SDL_DestroyTexture(this->texture);

	this->pix = nullptr;
	this->stext_page = nullptr;
	this->doc = nullptr;
	this->ctx = nullptr;
	this->texture = nullptr;
}

void Document::reload(){

    LoadingScreen ls(3);

    if (this->pix) fz_drop_pixmap(this->ctx, this->pix);
    if (this->stext_page) fz_drop_stext_page(this->ctx, this->stext_page);
    if (this->doc) fz_drop_document(this->ctx, this->doc);
	
    ls.update();
    ls.render();

    this->pix = nullptr;
	this->stext_page = nullptr;
	this->doc = nullptr;

    fz_try(this->ctx)
		this->doc = fz_open_document(this->ctx, FileManagerUtils::selected_path_file.c_str());
	fz_catch(this->ctx) {
		fz_report_error(this->ctx);
		fz_drop_context(this->ctx);
		debug_color(OmniBook::renderer,255,0,255);
		return;
	}

    ls.update();
    ls.render();

    this->loadPage(this->current_page);

    ls.update();
    ls.render();
}

Document::Document(const char*path){

	LoadingScreen loading(7);

    this->current_page = 1;
	this->num_pages = -1;
	this->zoom = 100.f;
	this->rotate = 0.f;
	this->scrollY = 0;
	this->valid = false;

	this->pix = nullptr;
	this->stext_page = nullptr;
	this->doc = nullptr;
	this->ctx = nullptr;
	this->texture = nullptr;

	loading.update();
	loading.render();

    this->ctx = fz_new_context(NULL, NULL, 16*1024*1024);
	if (!this->ctx) {
		debug_color(OmniBook::renderer,255,0,0);
		return;
	}

	loading.update();
	loading.render();

    fz_try(this->ctx)
		fz_register_document_handlers(this->ctx);
	fz_catch(this->ctx) {
		fz_report_error(this->ctx);
		fz_drop_context(this->ctx);
		debug_color(OmniBook::renderer,255,255,0);
		return;
	}

	loading.update();
	loading.render();
	
	fz_try(this->ctx)
		this->doc = fz_open_document(this->ctx, path);
	fz_catch(this->ctx) {
		fz_report_error(this->ctx);
		fz_drop_context(this->ctx);
		debug_color(OmniBook::renderer,255,0,255);
		return;
	}

	loading.update();
	loading.render();

    fz_try(this->ctx)
		this->num_pages = fz_count_pages(this->ctx, this->doc);
	fz_catch(this->ctx) {
		fz_report_error(this->ctx);
		fz_drop_document(this->ctx, this->doc);
		fz_drop_context(this->ctx);
		debug_color(OmniBook::renderer,255,255,255);
		return;
	}

	loading.update();
	loading.render();

	this->texture = SDL_CreateTexture(OmniBook::renderer, 
                                  SDL_PIXELFORMAT_RGB24, 
                                  SDL_TEXTUREACCESS_STREAMING, 
                                  W_SCREEN, H_SCREEN);

	loading.update();
	loading.render();
	
	if(!this->texture){
		debug_color(OmniBook::renderer,0,255,0);
		return;
	}
	this->valid = true;

	loading.update();
	loading.render();
}

void Document::nightToogle() {
	OmniBook::NightModeON = !OmniBook::NightModeON;

	fz_invert_pixmap(this->ctx, this->pix);

    unsigned char* sourcePtr = this->pix->samples + (int(this->scrollY) * this->pix->stride);
    SDL_UpdateTexture(this->texture, NULL, sourcePtr, this->pix->stride);
}

int Document::loadNextPage() {
	if (this->current_page < this->num_pages){
		this->scrollY = 0;
		this->loadPage(this->current_page + 1);
	}
	return this->current_page+1;
}

int Document::loadPreviousPage() {
	if (this->current_page > 0){
		this->scrollY = 0;
		this->loadPage(this->current_page - 1);
	}
	return this->current_page+1;
}

int Document::loadPage(int page) {
	if (page < 0)
		page = 0;
	else if (page == this->num_pages)
		page = this->num_pages-1;

	if(this->pix)
		fz_drop_pixmap(this->ctx, this->pix);
	if (this->stext_page)
		fz_drop_stext_page(this->ctx, this->stext_page);

	fz_page* page_ptr = fz_load_page(this->ctx, this->doc, page);
	fz_rect rect = fz_bound_page(this->ctx, page_ptr);

	float pageWidthPoints = rect.x1 - rect.x0;

	this->zoom = static_cast<float>(W_SCREEN) / pageWidthPoints * 100.f;
	this->ctm = fz_scale(this->zoom / 100, this->zoom / 100);
	this->ctm = fz_pre_rotate(this->ctm, this->rotate);

	fz_stext_options options = { FZ_STEXT_PRESERVE_WHITESPACE };
	this->stext_page = fz_new_stext_page_from_page(this->ctx, page_ptr, &options);

	fz_drop_page(this->ctx, page_ptr);

    fz_try(this->ctx)
		this->pix = fz_new_pixmap_from_page_number(this->ctx, this->doc, page, this->ctm, fz_device_rgb(this->ctx), 0);
	fz_catch(this->ctx) {
		fz_report_error(this->ctx);
		fz_drop_document(this->ctx, this->doc);
		fz_drop_context(this->ctx);
	}

	this->current_page = page;

    this->w = (unsigned int)fz_pixmap_width(this->ctx, this->pix);
	this->h = (unsigned int)fz_pixmap_height(this->ctx, this->pix);

	if (OmniBook::NightModeON){
		fz_invert_pixmap(this->ctx, this->pix);
	}

    unsigned char* sourcePtr = this->pix->samples + (int(this->scrollY) * this->pix->stride);
    SDL_UpdateTexture(this->texture, NULL, sourcePtr, this->pix->stride);

	return this->current_page+1;
}

void Document::render(){
	SDL_Rect rect;
	rect.x = 0;
	rect.w = W_SCREEN;
	rect.y = 0;
	rect.h = H_SCREEN;
    SDL_RenderCopy(OmniBook::renderer,this->texture,NULL,&rect);
}

void Document::scroll(float delta) {
	this->scrollY += delta;
	
	if (this->scrollY < 0) this->scrollY = 0;
    if (this->scrollY > (this->h - H_SCREEN))
        this->scrollY = this->h - H_SCREEN;

    unsigned char* sourcePtr = this->pix->samples + (int(this->scrollY) * this->pix->stride);
    SDL_UpdateTexture(this->texture, NULL, sourcePtr, this->pix->stride);
}

std::string Document::findWords(fz_rect&rect) {
	fz_matrix inv = fz_invert_matrix(this->ctm);
	rect.y0 += this->scrollY;
	rect.y1 += this->scrollY;
	rect = fz_transform_rect(rect, inv);

	char* testo = fz_copy_rectangle(this->ctx, this->stext_page, rect, 0);
	std::string ris = "";
	if (testo) {
		ris = testo;
		fz_free(this->ctx, testo);
		std::replace(ris.begin(), ris.end(), '\n', ' ');
		std::replace(ris.begin(), ris.end(), '\r', ' ');
	}
	return ris;
}

std::string Document::findWord(fz_point& mouse, fz_rect& sel) {
    if (!this->stext_page) return "";

    fz_matrix inv = fz_invert_matrix(this->ctm);
    
    fz_point p = { mouse.x, mouse.y + (float)this->scrollY };
    p = fz_transform_point(p, inv);
    
    fz_stext_block* block;
    fz_stext_line* line;
    fz_stext_char* ch;

    for (block = this->stext_page->first_block; block; block = block->next) {
        if (block->type != FZ_STEXT_BLOCK_TEXT) continue;

        for (line = block->u.t.first_line; line; line = line->next) {
            // Salta la riga se il punto è fuori verticalmente (ottimizzazione)
            if (p.y < line->bbox.y0 || p.y > line->bbox.y1) continue;

            std::string currentWord = "";
            bool pointInCurrentWord = false;
            // Inizializziamo il rettangolo della parola corrente come "vuoto"
            fz_rect wordRect = { 2147483647, 2147483647, -2147483647, -2147483647 };

            for (ch = line->first_char; ch; ch = ch->next) {
                if (ch->c <= 32) { // Spazio o invio
                    if (pointInCurrentWord) {
                        // HO TROVATO LA PAROLA!
                        sel = fz_transform_rect(wordRect, this->ctm);
                        sel.y0 -= this->scrollY;
                        sel.y1 -= this->scrollY;
                        return currentWord;
                    }
                
                    currentWord = "";
                    wordRect = { 2147483647, 2147483647, -2147483647, -2147483647 };
                    pointInCurrentWord = false;
                } else {
                    char utf8[10];
                    int len = fz_runetochar(utf8, ch->c);
                    utf8[len] = '\0';
                    currentWord += utf8;

                    wordRect.x0 = fz_min(wordRect.x0, ch->quad.ul.x);
                    wordRect.y0 = fz_min(wordRect.y0, ch->quad.ul.y);
                    wordRect.x1 = fz_max(wordRect.x1, ch->quad.lr.x);
                    wordRect.y1 = fz_max(wordRect.y1, ch->quad.lr.y);

                    // Test del tocco
                    if (p.x >= ch->quad.ul.x - 2.0f && p.x <= ch->quad.ur.x + 2.0f &&
                        p.y >= ch->quad.ul.y - 2.0f && p.y <= ch->quad.ll.y + 2.0f) {
                        pointInCurrentWord = true;
                    }
                }
            }
            // Gestione caso: la parola è l'ultima della riga
            if (pointInCurrentWord) {
                sel = fz_transform_rect(wordRect, this->ctm);
                sel.y0 -= this->scrollY;
                sel.y1 -= this->scrollY;
                return currentWord;
            }
        }
    }
    return "";
}

std::string Document::sendRequestForTraslation(std::string&data,std::string&from,std::string&to){
	std::string escapedText = url_encode(data);
    
    std::string url = fmt::format("http://translate.googleapis.com/translate_a/single?client=dict-chrome-ex&sl={}&tl={}&dt=t&q={}", from, to, escapedText);

    std::string response = "";
    
	int templateId = sceHttpCreateTemplate("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36", SCE_HTTP_VERSION_1_1, 1);
	if (templateId < 0) return "Errore: Init Template";

    int connId = sceHttpCreateConnectionWithURL(templateId, url.c_str(), SCE_TRUE);
    if (connId < 0) {
        sceHttpDeleteTemplate(templateId);
        return "Errore: Connessione";
    }

    int requestId = sceHttpCreateRequestWithURL(connId, SCE_HTTP_METHOD_GET, url.c_str(), 0);
    if (requestId < 0) {
        sceHttpDeleteConnection(connId);
        sceHttpDeleteTemplate(templateId);
        return "Errore: Richiesta";
    }
    
    sceHttpSetAutoRedirect(requestId, 1);
    
    sceHttpAddRequestHeader(requestId, "Accept", "application/json", SCE_HTTP_HEADER_ADD);
    sceHttpAddRequestHeader(requestId, "Accept-Language", "it-IT,it;q=0.9,en-US;q=0.8,en;q=0.7", SCE_HTTP_HEADER_ADD);
    
    // Invio richiesta
    int res = sceHttpSendRequest(requestId, NULL, 0);
    if (res < 0) {
        sceHttpDeleteRequest(requestId);
        sceHttpDeleteConnection(connId);
        sceHttpDeleteTemplate(templateId);
        return "Errore: Invio";
    }

    // deve essere 200
    int statusCode = 0;
    sceHttpGetStatusCode(requestId, &statusCode);
    if (statusCode != 200) {
        sceHttpDeleteRequest(requestId);
        sceHttpDeleteConnection(connId);
        sceHttpDeleteTemplate(templateId);
        return fmt::format("Errore HTTP: {}", statusCode);
    }

    unsigned char readBuffer[4096];
    int readBytes = 0;
    while ((readBytes = sceHttpReadData(requestId, readBuffer, sizeof(readBuffer))) > 0) {
        response.append((char*)readBuffer, readBytes);
    }

    sceHttpDeleteRequest(requestId);
    sceHttpDeleteConnection(connId);
    sceHttpDeleteTemplate(templateId);
    
    if (!response.empty()) {
        try {
            auto j = json::parse(response);
            std::string testoTradotto = "";

            // Struttura Google: [[["tradotto", "originale", ...]]]
            if (j.is_array() && j.size() > 0 && j[0].is_array()) {
                for (auto& element : j[0]) {
                    if (element.is_array() && element[0].is_string()) {
                        testoTradotto += element[0].get<std::string>();
                    }
                }
                return testoTradotto;
            }
        }
        catch (json::parse_error& e) {
            return fmt::format("Errore: Formato non JSON -> {}",response);
        }
    }

    return "Risposta vuota";
}

std::string Document::translate(std::string&data,std::string&from,std::string&to){
	return this->sendRequestForTraslation(data,from,to);
}

std::string Document::translate(fz_rect& rect, std::string& from, std::string& to) { 
    std::string data = this->findWords(rect);
    if (data.empty()) return "";

    return this->sendRequestForTraslation(data,from,to);
}

size_t WriteCb(void* contents, size_t size, size_t nmemb, std::string* s) {
	size_t newLength = size * nmemb;
	s->append((char*)contents, newLength);
	return newLength;
}