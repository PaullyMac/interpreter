ParseTreeNode *parse_equality_exp() {
    ParseTreeNode *left = parse_factor();

    // Only go through the parsing of OR's if it's not a lone factor 
    if (current_token < num_tokens && tokens[current_token].type == equal || current_token < num_tokens && tokens[current_token].type == not_equal) { // check operator if equal or not
        ParseTreeNode *node = create_equality_exp_node();

        // Add the factor (left operand) to the logical or node 
        add_child(node, left); 

        
        while (current_token < num_tokens && tokens[current_token].type == equal || not_equal) { 
            add_child(node, match_and_create_node(tokens[current_token].type, "Operator")); // if statement pag kukunin type // check if token type is equal or not equal  if tokentype is equal == equal


            // Add the factor (right operand) to the logical or node
            ParseTreeNode *right = parse_factor(); //next
            add_child(node, right);

            // Assure left-associativity, create a new node above the previous if there are more ORs
            if (current_token < num_tokens && tokens[current_token].type == equal || not_equal) {
              ParseTreeNode *new_node = create_equality_exp_node();

              // The previous OR node we built is added as a child to a new OR node
            add_child(new_node, node);
            node = new_node;
            }
        }
        return node;
    }

    // Return the factor directly if there's no OR
    return left; 
}